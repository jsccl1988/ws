#include <wspp/util/template_renderer.hpp>
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/regex.hpp>

using namespace std ;

namespace wspp {


namespace mustache {

struct Tag {

    enum Type { None, EscapedSubstitution, RawSubstitutionAmpersand, RawSubstitutionCurlyBracket, Comment, Block, Extension, SectionBegin, SectionEnd, InvertedSectionBegin, Partial } ;

    Tag(): type_(None)  {}
    string name_ ;
    Type type_ ;
};

struct ContextStack {

    Variant top() const { return stack_.back() ; }

    void push(Variant ctx) {
        stack_.push_back(ctx) ;
    }

    void pop() { stack_.pop_back() ; }

    Variant find(const string &item) {
        for ( auto it = stack_.rbegin() ; it != stack_.rend() ; ++it ) {
            Variant v = it->at(item) ;
            if ( !v.isNull() ) return v ;
        }

        return Variant() ;
    }


    std::deque<Variant> stack_ ;
};

struct Node {

    typedef boost::shared_ptr<Node> Ptr ;

    enum Type { RawText, Section, Comment, Substitution, Partial, Extension, Block } ;

    virtual Type type() const = 0;
    virtual void eval(ContextStack &ctx, string &res) const = 0 ;
};

struct RawTextNode: public Node {

    typedef boost::shared_ptr<RawTextNode> Ptr ;

    RawTextNode(const string &text): text_(text) {}

    void eval(ContextStack &ctx, string &res) const override {
        res.append(text_) ;
    }

    Type type() const override { return RawText ; }

    string text_ ;
};

struct BlockNode ;

struct ContainerNode: public Node {
    typedef boost::shared_ptr<ContainerNode> Ptr ;

    ContainerNode(const string &name): name_(name) {}

    string name_ ;
    vector<Node::Ptr> children_ ;
    map<string, Node::Ptr> blocks_ ;
};

struct SectionNode: public ContainerNode {

    typedef boost::shared_ptr<SectionNode> Ptr ;

    SectionNode(const string &name, bool is_inverted = false): ContainerNode(name), is_inverted_(is_inverted) {}

    Type type() const override { return Section ; }
    void eval(ContextStack &ctx, string &res) const override {
        Variant val = ctx.find(name_) ;
        if ( !is_inverted_ ) {
            if ( val.isFalse() ) return ; // either does not exist or set to explicitly set to Null, false, empty array
            else if ( val.isArray() ) {
                for( uint k=0 ; k<val.length() ; k++ ) {
                    Variant v = val.at(k) ;
                    ctx.push(v) ;
                    for( auto &c: children_) {
                        c->eval(ctx, res) ;
                    }
                    ctx.pop() ;
                }
            }
            else if ( val.isObject() ) { //push context and evaluate children
                ctx.push(val) ;
                for( auto &c: children_) {
                    c->eval(ctx, res)  ;
                }
                ctx.pop() ;
            }
            else {
                for( auto &c: children_) {
                    c->eval(ctx, res)  ;
                }
            }
        }
        else {
            if ( val.isFalse() ) {
                for( auto &c: children_) {
                    c->eval(ctx, res)  ;
                }
            }
        }
    }

    bool is_inverted_ ;
};

struct BlockNode: public ContainerNode {

    typedef boost::shared_ptr<BlockNode> Ptr ;

    BlockNode(const string &name): ContainerNode(name) {}

    Type type() const override { return Block ; }
    void eval(ContextStack &ctx, string &res) const override {
       for( auto &c: children_) {
           c->eval(ctx, res) ;
       }
    }
};


static string escape(const string &src) {
    string buffer ;
    for ( char c: src ) {
        switch(c) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.push_back(c);          break;
        }
    }

    return buffer ;
}

struct SubstitutionNode: public Node {

    typedef boost::shared_ptr<SubstitutionNode> Ptr ;

    SubstitutionNode(const string &var, bool is_escaped): var_(var), is_escaped_(is_escaped) {}
    void eval(ContextStack &ctx, string &res) const override {

        string sub ;
        Variant val ;
        if ( var_ == "." )
            val = ctx.top() ;
        else
            val = ctx.find(var_) ;

        if ( val.isNull() ) return ;
        else if ( val.isValue() )
            sub = val.toString() ;
        else return ;

        res.append((is_escaped_ ) ? escape(sub) : sub ) ;
    }

    Type type() const override { return Substitution ; }

    string var_ ;
    bool is_escaped_ ;
};




class Cache {
public:

    typedef std::pair<time_t, SectionNode::Ptr> Entry ;

    void add(const string &key, time_t ts, SectionNode::Ptr &val) {
        boost::mutex::scoped_lock lock(guard_);
        compiled_.insert({key, std::make_pair(ts, val)}) ;
    }

    Entry fetch(const string &key) {
        boost::mutex::scoped_lock lock(guard_);
        auto it = compiled_.find(key) ;
        if ( it != compiled_.end() ) return it->second ;
        else return Entry(0, nullptr) ;
    }

private:
    map<string, Entry> compiled_ ;
    boost::mutex guard_ ;

};

class Parser {
public:

    Parser(const Partials &partials, const string &folder, bool caching): idx_(0), partials_(partials), root_folder_(folder), caching_(caching) {
    }

    SectionNode::Ptr parse(const string &src) {
        if ( !src.empty() && src.at(0) == '@' ) { // pointing to a file
            string filepath = src.substr(1) ;

            using namespace boost::filesystem ;

            path p(root_folder_) ;
            p /= filepath ;

            if ( !exists(p) ) return nullptr ;
            std::time_t t = last_write_time( p ) ;

            static Cache g_cache ;

            if ( caching_ ) {
                auto stored = g_cache.fetch(p.string()) ;
                if ( stored.second && stored.first >= t ) return stored.second ;
            }

            std::ifstream strm(p.string()) ;

            if ( strm ) {
                string contents((istreambuf_iterator<char>(strm)), istreambuf_iterator<char>());
                auto compiled = parseString(contents) ;
                if ( caching_ ) g_cache.add(p.string(), t, compiled) ;
                return compiled ;
            }
            else return nullptr ;
        }
        else
            return parseString(src) ;
    }

    SectionNode::Ptr parseString(const string &src) ;

private:


    bool expect(const string &, char c) ;
    string eatTag(const string &) ;
    void parseTag(const string &src, Tag &tag) ;
    bool nextTag(const string &src, string &raw, Tag &tag) ;
    bool parsePartialTag(const string &tag, string &name, Dictionary &args) ;


private:
    friend class PartialNode ;
    friend class ExtensionNode ;

    uint idx_ ;
    const Partials &partials_ ;
    const string &root_folder_ ;
    bool caching_ ;
};

bool Parser::parsePartialTag(const string &tag, string &name, Dictionary &args) {
    name = tag ;
    return true ;
}

bool Parser::expect(const string &src, char c) {
    if ( idx_ < src.length() ) {
        if ( src[idx_] == c ) {
            ++idx_ ;
            return true ;
        } else return false ;
    } else return false ;
}

string Parser::eatTag(const string &src) {
    string res ;

    uint nc = src.length() ;

    while ( idx_ < nc ) {
        if ( expect(src, '}') ) {
            if ( expect(src, '}') ) {
                return res ;
            }
            else if ( idx_ < nc ) res.push_back(src[idx_++]) ;
        }
        else if ( idx_ < nc ) res.push_back(src[idx_++]) ;
    }
}

void Parser::parseTag(const string &src, Tag &tag) {

    uint nc = src.length() ;

    if ( idx_ >= nc ) return ;

    switch ( src[idx_] ) {
    case '#':  tag.type_ = Tag::SectionBegin ; ++idx_ ; break ;
    case '/':  tag.type_ = Tag::SectionEnd ; ++idx_ ; break ;
    case '^':  tag.type_ = Tag::InvertedSectionBegin ; ++idx_ ; break ;
    case '&':  tag.type_ = Tag::RawSubstitutionAmpersand ; ++idx_ ; break ;
    case '{':  tag.type_ = Tag::RawSubstitutionCurlyBracket ; ++idx_ ; break ;
    case '!':  tag.type_ = Tag::Comment ; ++idx_ ; break ;
    case '>':  tag.type_ = Tag::Partial ; ++idx_ ; break ;
    case '<':  tag.type_ = Tag::Extension ; ++idx_ ; break ;
    case '$':  tag.type_ = Tag::Block ; ++idx_ ; break ;
    default: tag.type_ = Tag::EscapedSubstitution ; break ;
    }

    // eat comment
    if ( tag.type_ == Tag::Comment ) {
        uint level = 0 ;
        while ( idx_ < nc ) {
            if ( expect(src, '}') ) {
                if ( expect(src, '}') ) {
                    if ( level == 0 ) break ;
                    else level -- ;
                }
                else ++idx_ ;
            }
            else if ( expect(src, '{') ) {
                if ( expect(src, '{'))
                    level++ ;
                else ++idx_ ;
            }
            else ++idx_ ;
        }

        return ;
    }

    while ( idx_ < nc ) {
        if ( expect(src, '}') ) {
            if ( expect(src, '}') ) {
                if ( tag.type_ == Tag::RawSubstitutionCurlyBracket )
                    expect(src, '}') ; // just eat the character otherwise we silently continue
                return ;
            }
            else if ( idx_ < nc ) {
                if ( !isspace(src[idx_]) )
                    tag.name_.push_back(src[idx_]) ;
                ++idx_ ;
            }
        }
        else if ( idx_ < nc ) {
            if ( !isspace(src[idx_]) )
                tag.name_.push_back(src[idx_]) ;
            ++idx_ ;
        }
    }
}

bool Parser::nextTag(const string &src, string &raw, Tag &tag) {
    uint nc = src.length() ;

    while ( idx_ < nc ) {
        if ( expect(src, '{') ) {
            if ( expect(src, '{') ) {

                parseTag(src, tag) ;

                if ( tag.type_ == Tag::Comment ) continue ;
                return true ;
            }
            else --idx_ ;
        }

        if ( idx_ < nc ) raw.push_back(src[idx_++]) ;
    }

    return false ;
}

struct PartialNode: public Node {

    typedef boost::shared_ptr<PartialNode> Ptr ;

    // inherit parent parameters
    PartialNode(const string &name, const Dictionary &args, const Parser &context):
        key_(name), args_(args), context_(context) {
    }

    void eval(ContextStack &ctx, string &res) const override {

        static boost::regex partial_rx("%([^%]+)%") ;

        string key = boost::regex_replace(key_, partial_rx, [&](const boost::smatch &matches) -> string {
                    string param = matches[1] ;
                    if ( !param.empty() ) {
                        Variant p = ctx.find(param) ;
                        if ( p.isValue() ) return p.toString() ;
                        else return string() ;
                    }
                    else return string() ;

        }) ;

        string partial_src ;

        // check if the key has been declared in the partials map
        auto p = context_.partials_.find(key) ;
        if ( p != context_.partials_.end() )
            partial_src = p->second ;
        else if ( key.at(0) == '@' ) // else if the key starts with @ it points to a file
            partial_src = key ;

        if ( !partial_src.empty() ) {
            Parser parser(context_.partials_, context_.root_folder_, context_.caching_) ;
            auto ast = parser.parse(partial_src) ;
            if ( ast ) ast->eval(ctx, res) ;
        }
    }

    Type type() const override { return Partial ; }

    string key_ ;
    Dictionary args_ ;
    const Parser &context_ ;
};

struct ExtensionNode: public ContainerNode {

    typedef boost::shared_ptr<ExtensionNode> Ptr ;

    // inherit parent parameters
    ExtensionNode(const string &name, const Parser &context): context_(context), ContainerNode(name) {
    }

    void eval(ContextStack &ctx, string &res) const override {

        // load base node

        string partial_src ;

        // check if the key has been declared in the partials map
        auto p = context_.partials_.find(name_) ;
        if ( p != context_.partials_.end() )
            partial_src = p->second ;
        else if ( name_.at(0) == '@' ) // else if the key starts with @ it points to a file
            partial_src = name_ ;

        if ( !partial_src.empty() ) {
            // parse base node
            Parser parser(context_.partials_, context_.root_folder_, context_.caching_) ;
            auto ast = parser.parse(partial_src) ;
            if ( ast ) {
                // replace parent blocks with child blocks

                for( auto &c: ast->children_ ) {
                    if ( c->type() == Node::Block ) {
                        BlockNode::Ptr block = boost::dynamic_pointer_cast<BlockNode>(c) ;
                        auto it = blocks_.find(block->name_) ;
                        if ( it == blocks_.end() ) {
                            block->eval(ctx, res) ; // render default block
                        }
                        else
                            it->second->eval(ctx, res) ; // render child block
                    }
                    else
                        c->eval(ctx, res) ;
                }
            }
        }

    }

    Type type() const override { return Extension ; }
    const Parser &context_ ;

};

SectionNode::Ptr Parser::parseString(const string &src) {

    SectionNode::Ptr root(new SectionNode("$root")) ;

    deque<ContainerNode::Ptr> stack ;

    stack.push_back(root) ;

    while (!stack.empty()) {
        ContainerNode::Ptr parent = stack.back() ;

        string raw ;
        Tag tag ;
        bool res = nextTag(src, raw, tag)  ;

        string name ;
        Dictionary args ;

        if ( !raw.empty() ) {
            parent->children_.push_back(boost::make_shared<RawTextNode>(raw)) ;
        }

        if ( tag.type_ == Tag::SectionBegin  ) {
            SectionNode::Ptr new_section(new SectionNode(tag.name_)) ;
            parent->children_.push_back(new_section) ;
            stack.push_back(new_section) ;
        }
        else if ( tag.type_ == Tag::InvertedSectionBegin  ) {
            SectionNode::Ptr new_section(new SectionNode(tag.name_, true)) ;
            parent->children_.push_back(new_section) ;
            stack.push_back(new_section) ;
        }
        else if ( tag.type_ == Tag::SectionEnd ) {
            if ( tag.name_ != parent->name_ ) return nullptr ;
            else stack.pop_back() ;
        }
        else if ( tag.type_ == Tag::RawSubstitutionAmpersand || tag.type_ == Tag::RawSubstitutionCurlyBracket ) {
            parent->children_.push_back(boost::make_shared<SubstitutionNode>(tag.name_, false)) ;
        }
        else if ( tag.type_ == Tag::EscapedSubstitution ) {
            parent->children_.push_back(boost::make_shared<SubstitutionNode>(tag.name_, true)) ;
        }
        else if ( tag.type_ == Tag::Partial ) {
            if ( parsePartialTag(tag.name_, name, args) )
                parent->children_.push_back(boost::make_shared<PartialNode>(name, args, *this)) ;
        }
        else if ( tag.type_ == Tag::Extension ) {

            ExtensionNode::Ptr new_section(new ExtensionNode(tag.name_, *this)) ;
            parent->children_.push_back(new_section) ;
            stack.push_back(new_section) ;
        }
        else if ( tag.type_ == Tag::Block ) {
            BlockNode::Ptr new_block(new BlockNode(tag.name_)) ;
            parent->children_.push_back(new_block) ;
            parent->blocks_.insert({tag.name_, new_block}) ;
            stack.push_back(new_block) ;
        }

        if ( !res ) stack.pop_back() ;

    }

    return root ;
}

}

string TemplateRenderer::render(const string &src, const Variant &ctx, const Partials &partials) {
    using namespace mustache ;

    Parser parser(partials, root_folder_, caching_) ;

    string res ;
    SectionNode::Ptr ast = parser.parse(src) ;

    if ( ast ) {
        Variant rc(Variant::Object{{"$root", ctx}}) ;
        ContextStack stack ;
        stack.push(rc) ;
        ast->eval(stack, res) ;
    }

    return res ;

}





}
