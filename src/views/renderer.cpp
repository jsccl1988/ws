#include <wspp/views/renderer.hpp>
#include <wspp/util/filesystem.hpp>

#include <sstream>
#include <fstream>
#include <string>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

using namespace std ;
using namespace wspp::util ;

namespace wspp { namespace web {

FileSystemTemplateLoader::FileSystemTemplateLoader(const std::initializer_list<string> &root_folders, const string &suffix):
    root_folders_(root_folders), suffix_(suffix) {
}

string FileSystemTemplateLoader::load(const string &key) {

    using namespace boost::filesystem ;

    for ( const string &r: root_folders_ ) {
        path p(r) ;

        p /= key + suffix_;

        if ( !exists(p) ) continue ;

        return readFileToString(p.string()) ;

    }

    return string() ;
}



struct Tag {

    enum Type { None, EscapedSubstitution, RawSubstitutionAmpersand, RawSubstitutionCurlyBracket, Comment, Block, Extension, SectionBegin, SectionEnd, InvertedSectionBegin, Partial } ;

    Tag(): type_(None)  {}
    string name_ ;
    Type type_ ;
};



struct Node {

    typedef boost::shared_ptr<Node> Ptr ;

    enum Type { RawText, Section, Comment, Substitution, Partial, Extension, Block, Helper } ;

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

    typedef SectionNode::Ptr Entry ;

    void add(const string &key, SectionNode::Ptr &val) {
        boost::mutex::scoped_lock lock(guard_);
        compiled_.insert({key, val}) ;
    }

    Entry fetch(const string &key) {
        boost::mutex::scoped_lock lock(guard_);
        auto it = compiled_.find(key) ;
        if ( it != compiled_.end() ) return it->second ;
        else return nullptr ;
    }

private:
    map<string, Entry> compiled_ ;
    boost::mutex guard_ ;

};

class Parser {
public:

    Parser(const boost::shared_ptr<TemplateLoader> &loader,
           const map<string, TemplateRenderer::BlockHelper> &block_helpers,
           const map<string, TemplateRenderer::ValueHelper> &value_helpers,
           bool caching): idx_(0), loader_(loader), block_helpers_(block_helpers), value_helpers_(value_helpers), caching_(caching) {
    }

    SectionNode::Ptr parse(const string &key) {
        static Cache g_cache ;

        if ( key.empty() ) return nullptr ;

        if ( caching_ ) {
            auto stored = g_cache.fetch(key) ;
            if ( stored ) return stored ;
        }

        string src = loader_->load(key) ;

        if  ( src.empty() ) return nullptr ;

        auto compiled = parseString(src) ;
        if ( caching_ ) g_cache.add(key, compiled) ;

        return compiled ;
    }

    SectionNode::Ptr parseString(const string &src) ;

    struct Arg {
        Arg(const string &key, const string &val, bool is_literal): key_(key), val_(val), is_literal_(is_literal) {}

        string key_ ;
        string val_ ;
        bool is_literal_ ;
    };

    static Variant::Object getDictionaryArgs(const vector<Arg> &args, ContextStack &stack) {
        Variant::Object res ;

        for( auto &a: args ) {
            if ( a.key_.empty() ) continue ;
            string value = ( !a.is_literal_ ) ? stack.find(a.val_).toString() : a.val_ ;
            res.insert({a.key_, value}) ;
        }
        return res ;
    }

    static Variant::Array getParams(const vector<Arg> &args, ContextStack &ctx) {
        Variant::Array res ;

        for( auto &a: args ) {
            if ( !a.key_.empty() ) continue ;
            if ( a.is_literal_ ) res.emplace_back(a.val_) ;
            else res.emplace_back(ctx.find(a.val_)) ;
        }
        return res ;
    }

private:


    bool expect(const string &, char c) ;
    string eatTag(const string &) ;
    void parseTag(const string &src, Tag &tag) ;
    bool nextTag(const string &src, string &raw, Tag &tag, int &cursor) ;
    bool parseComplexTag(const string &tag, string &name, vector<Arg> &args) ;
    bool parseSimpleTag(const string &tag, string &name) ;


private:
    friend class PartialNode ;
    friend class ExtensionNode ;

    uint idx_ ;
    boost::shared_ptr<TemplateLoader> loader_ ;
    bool caching_ ;
    const map<string, TemplateRenderer::BlockHelper> &block_helpers_ ;
    const map<string, TemplateRenderer::ValueHelper> &value_helpers_ ;
};

bool Parser::parseComplexTag(const string &tag, string &name, vector<Arg> &args) {
    static boost::regex rx_args(R"%((\w+)\s*=\s*"([^"]*)|(\w+)|"([^"]*)")%") ;
    static boost::regex rx_name(R"(^\s*([^\s\()]+))") ;

    boost::smatch tmatch ;
    if ( !boost::regex_search(tag, tmatch, rx_name) ) return false ;
    name = tmatch[1] ;

    std::string::const_iterator start = tmatch[0].second;
    boost::sregex_iterator it(start, tag.end(), rx_args), end ;

    while ( it != end ) {

        string key = (*it)[1] ;
        string val = (*it)[2] ;
        string param = (*it)[3] ;
        string literal = (*it)[4] ;
        if ( !key.empty() )
            args.emplace_back(key, val, true) ;
        else if ( !param.empty() )
            args.emplace_back(string(), param, false) ;
        else if ( !literal.empty() )
            args.emplace_back(string(), literal, true) ;

        ++ it ;
    }

    return true ;
}

bool Parser::parseSimpleTag(const string &tag, string &name) {

    name = boost::trim_copy(tag) ;
    return !name.empty() ;

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
                //  if ( !isspace(src[idx_]) )
                tag.name_.push_back(src[idx_]) ;
                ++idx_ ;
            }
        }
        else if ( idx_ < nc ) {
            //      if ( !isspace(src[idx_]) )
            tag.name_.push_back(src[idx_]) ;
            ++idx_ ;
        }
    }
}

bool Parser::nextTag(const string &src, string &raw, Tag &tag, int &cursor) {
    uint nc = src.length() ;

    while ( idx_ < nc ) {
        if ( expect(src, '{') ) {
            if ( expect(src, '{') ) {

                cursor = idx_ - 2 ;
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
    PartialNode(const string &name, const vector<Parser::Arg> &args, const Parser &context):
        key_(name), args_(args), context_(context) {
    }

    void eval(ContextStack &ctx, string &res) const override {

        string key ;

        Variant v = ctx.find(key_) ;
        if ( v.isValue() )
            key = v.toString() ;

        if ( key.empty() ) key = key_ ;

        Parser parser(context_.loader_, context_.block_helpers_, context_.value_helpers_, context_.caching_) ;
        auto ast = parser.parse(key) ;
        if ( ast ) {
            ctx.push(Parser::getDictionaryArgs(args_, ctx)) ;
            ast->eval(ctx, res) ;
            ctx.pop() ;
        }
    }

    Type type() const override { return Partial ; }

    string key_ ;
    vector<Parser::Arg> args_ ;
    const Parser &context_ ;
};

struct BlockHelperNode: public Node {


    typedef boost::shared_ptr<BlockHelperNode> Ptr ;

    // inherit parent parameters
    BlockHelperNode(const string &name, const vector<Parser::Arg> &args, TemplateRenderer::BlockHelper helper): key_(name), args_(args), helper_(helper) {}

    void eval(ContextStack &ctx, string &res) const override {

        ctx.push(Parser::getDictionaryArgs(args_, ctx)) ;
        Variant::Array params = Parser::getParams(args_, ctx) ;
        string part = helper_(content_, ctx, params) ;
        ctx.pop() ;
        res.append(part) ;

    }

    void setContent(const std::string &content) {
        content_ = content ;
    }

    Type type() const override { return Helper ; }

    string key_, content_ ;
    TemplateRenderer::BlockHelper helper_ ;
    vector<Parser::Arg> args_ ;

};

struct ValueHelperNode: public Node {

    typedef boost::shared_ptr<ValueHelperNode> Ptr ;

    // inherit parent parameters
    ValueHelperNode(const string &name, const vector<Parser::Arg> &args, TemplateRenderer::ValueHelper helper, bool escape): key_(name), args_(args), helper_(helper), escape_(escape) {}

    void eval(ContextStack &ctx, string &res) const override {

        ctx.push(Parser::getDictionaryArgs(args_, ctx)) ;
        Variant::Array params = Parser::getParams(args_, ctx) ;
        pair<bool, string> part = helper_(ctx, params) ;
        ctx.pop() ;

        if ( escape_ && !part.first ) res.append(escape(part.second)) ;
        else res.append(part.second) ;
    }

    Type type() const override { return Helper ; }

    string key_ ;
    TemplateRenderer::ValueHelper helper_ ;
    vector<Parser::Arg> args_ ;
    bool escape_ ;

};

struct ExtensionNode: public ContainerNode {

    typedef boost::shared_ptr<ExtensionNode> Ptr ;

    // inherit parent parameters
    ExtensionNode(const string &name, const vector<Parser::Arg> &args, const Parser &context): context_(context), ContainerNode(name) {
    }

    void eval(ContextStack &ctx, string &res) const override {

        // load base node

        Parser parser(context_.loader_, context_.block_helpers_, context_.value_helpers_, context_.caching_) ;
        auto ast = parser.parse(name_) ;
        if ( ast ) {
            // replace parent blocks with child blocks

            ctx.push(Parser::getDictionaryArgs(args_, ctx)) ;
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
            ctx.pop() ;
        }


    }

    Type type() const override { return Extension ; }
    const Parser &context_ ;
    vector<Parser::Arg> args_ ;

};

SectionNode::Ptr Parser::parseString(const string &src) {

    SectionNode::Ptr root(new SectionNode("$root")) ;

    deque<ContainerNode::Ptr> stack ;

    stack.push_back(root) ;

    int s_cursor = 0, e_cursor ;
    BlockHelperNode::Ptr helper_node ;

    while (!stack.empty()) {
        ContainerNode::Ptr parent = stack.back() ;

        string raw ;
        Tag tag ;
        bool res = nextTag(src, raw, tag, e_cursor)  ;

        string name ;
        vector<Arg> args ;

        if ( helper_node ) { // just skip until we found a valid closing tag
            if ( tag.type_ == Tag::SectionEnd ) {
                if ( parseSimpleTag(tag.name_, name)) {
                    if ( name == helper_node->key_ ) {
                        helper_node->content_ = src.substr(s_cursor, e_cursor-s_cursor) ;
                        helper_node = nullptr ;
                    }
                }
            }
        } else {
            if ( !raw.empty() ) {
                parent->children_.push_back(boost::make_shared<RawTextNode>(raw)) ;
            }

            if ( tag.type_ == Tag::SectionBegin  ) {
                if ( parseComplexTag(tag.name_, name, args)) {
                    auto it = block_helpers_.find(name) ;
                    if ( it != block_helpers_.end()) { // if it is a registered helper
                        helper_node.reset(new BlockHelperNode(name, args, it->second)) ;
                        parent->children_.push_back(helper_node) ;
                        s_cursor = idx_ ;
                    }
                    else {
                        SectionNode::Ptr new_section(new SectionNode(name)) ;
                        parent->children_.push_back(new_section) ;
                        stack.push_back(new_section) ;
                    }
                }
            }
            else if ( tag.type_ == Tag::InvertedSectionBegin  ) {
                if ( parseSimpleTag(tag.name_, name)) {
                    SectionNode::Ptr new_section(new SectionNode(name, true)) ;
                    parent->children_.push_back(new_section) ;
                    stack.push_back(new_section) ;
                }
            }
            else if ( tag.type_ == Tag::SectionEnd ) {
                if ( parseSimpleTag(tag.name_, name)) {
                    if ( name != parent->name_ ) return nullptr ;
                    else stack.pop_back() ;
                }
            }
            else if ( tag.type_ == Tag::RawSubstitutionAmpersand || tag.type_ == Tag::RawSubstitutionCurlyBracket ) {
                if ( parseComplexTag(tag.name_, name, args)) {
                        auto it = value_helpers_.find(name) ;
                        if ( it != value_helpers_.end()) { // if it is a registered helper
                           parent->children_.push_back(boost::make_shared<ValueHelperNode>(name, args, it->second, false)) ;
                        } else
                            parent->children_.push_back(boost::make_shared<SubstitutionNode>(name, false)) ;
                }
            }
            else if ( tag.type_ == Tag::EscapedSubstitution ) {
                if ( parseComplexTag(tag.name_, name, args)) {
                    auto it = value_helpers_.find(name) ;
                    if ( it != value_helpers_.end()) { // if it is a registered helper
                       parent->children_.push_back(boost::make_shared<ValueHelperNode>(name, args, it->second, true)) ;
                    } else
                        parent->children_.push_back(boost::make_shared<SubstitutionNode>(name, true)) ;
                }

            }
            else if ( tag.type_ == Tag::Partial ) {
                if ( parseComplexTag(tag.name_, name, args) )
                    parent->children_.push_back(boost::make_shared<PartialNode>(name, args, *this)) ;
            }
            else if ( tag.type_ == Tag::Extension ) {
                if ( parseComplexTag(tag.name_, name, args) ) {
                    ExtensionNode::Ptr new_section(new ExtensionNode(name, args, *this)) ;
                    parent->children_.push_back(new_section) ;
                    stack.push_back(new_section) ;
                }
            }
            else if ( tag.type_ == Tag::Block ) {
                if ( parseSimpleTag(tag.name_, name)) {
                    BlockNode::Ptr new_block(new BlockNode(name)) ;
                    parent->children_.push_back(new_block) ;
                    parent->blocks_.insert({name, new_block}) ;
                    stack.push_back(new_block) ;
                }
            }
        }

        if ( !res ) stack.pop_back() ;

    }

    return root ;
}


string TemplateRenderer::render(const string &src, const Variant &ctx) {

    Parser parser(loader_,  block_helpers_, value_helpers_, caching_) ;

    string res ;
    SectionNode::Ptr ast = parser.parse(src) ;

    if ( ast ) {
        Variant rc(Variant::Object{{"$root", true}}) ;
        ContextStack stack ;
        stack.push(rc) ;
        stack.push(ctx) ;
        ast->eval(stack, res) ;
    }

    return res ;
}

string TemplateRenderer::renderString(const string &src, ContextStack &ctx) {

    Parser parser(loader_,  block_helpers_, value_helpers_, caching_) ;

    string res ;
    SectionNode::Ptr ast = parser.parseString(src) ;

    if ( ast ) {
        ast->eval(ctx, res) ;
    }

    return res ;
}

void TemplateRenderer::registerBlockHelper(const string &name, TemplateRenderer::BlockHelper helper)
{
    block_helpers_.insert({name, helper}) ;
}

void TemplateRenderer::registerValueHelper(const string &name, TemplateRenderer::ValueHelper helper)
{
    value_helpers_.insert({name, helper}) ;
}

void TemplateRenderer::registerDefaultHelpers() {
    registerValueHelper("render", [&](ContextStack &ctx, Variant::Array params) -> pair<bool, string> {
        Variant key = params.at(0) ;
        if ( key.isNull() ) return make_pair(true, string()) ;
        return make_pair(true, renderString(key.toString(), ctx)) ;
    }) ;
}


} // namespace web
               } // namespace wspp
