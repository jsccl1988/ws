#include <wspp/util/template_renderer.hpp>
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/mutex.hpp>


using namespace std ;

namespace wspp {


namespace mustache {

struct Tag {

    enum Type { None, EscapedSubstitution, RawSubstitutionAmpersand, RawSubstitutionCurlyBracket, Comment, SectionBegin, SectionEnd, InvertedSectionBegin, Partial } ;

    Tag(): type_(None)  {}
    string name_ ;
    Type type_ ;
};

struct Node {

    typedef boost::shared_ptr<Node> Ptr ;

    enum Type { RawText, Section, Comment, Substitution, Partial } ;

    virtual Type type() const = 0;
    virtual void eval(const Variant &ctx, string &res) const = 0 ;
};

struct RawTextNode: public Node {

    typedef boost::shared_ptr<RawTextNode> Ptr ;

    RawTextNode(const string &text): text_(text) {}

    void eval(const Variant &ctx, string &res) const override {
        res.append(text_) ;
    }

    Type type() const override { return RawText ; }

    string text_ ;
};

struct SectionNode: public Node {

    typedef boost::shared_ptr<SectionNode> Ptr ;

    SectionNode(const string &name, bool is_inverted = false): name_(name), is_inverted_(is_inverted) {}

    Type type() const override { return Section ; }
    void eval(const Variant &ctx, string &res) const override {
        Variant val = ctx.at(name_) ;
        if ( !is_inverted_ ) {
            if ( val.isNull() || val.length() == 0 ) return ; // either does not exist or set to explicitly set to Null
            else if ( val.isArray() && val.length() > 0) {
                for( uint k=0 ; k<val.length() ; k++ ) {
                    Variant v = val.at(k) ;
                    for( auto &c: children_) {
                        c->eval(v, res) ;
                    }
                }
            }
            else if ( val.isObject() ) {
                for( auto &c: children_) {
                    c->eval(val, res)  ;
                }
            }
        }
        else {
            if ( val.isNull() || val.length() == 0 ) {
                for( auto &c: children_) {
                    c->eval(Variant(), res)  ;
                }
            }
        }
    }

    string name_ ;
    bool is_inverted_ ;
    vector<Node::Ptr> children_ ;
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
    void eval(const Variant &ctx, string &res) const override {

        string sub ;
        if ( ctx.isObject() ) {
            auto val = ctx.at(var_) ;
            if ( val.isNull() ) return ;
            else if ( val.isValue() )
                sub = val.toString() ;
            else return  ;
        }
        else return ;

        res.append((is_escaped_ ) ? escape(sub) : sub ) ;
    }

    Type type() const override { return Substitution ; }

    string var_ ;
    bool is_escaped_ ;
};

class Cache {
public:

    void add(const string &key, SectionNode::Ptr &val) {
        boost::mutex::scoped_lock lock(guard_);
        compiled_.insert({key, val}) ;
    }

    SectionNode::Ptr fetch(const string &key) {
        boost::mutex::scoped_lock lock(guard_);
        auto it = compiled_.find(key) ;
        if ( it != compiled_.end() ) return it->second ;
        else return nullptr ;
    }

private:
    map<string, SectionNode::Ptr> compiled_ ;
    boost::mutex guard_ ;

};

class Parser {
public:

    Parser(const Partials &partials, const string &folder): idx_(0), partials_(partials), root_folder_(folder) {
    }

    SectionNode::Ptr parse(const string &src) {
        if ( !src.empty() && src.at(0) == '@' ) { // pointing to a file
            string filepath = src.substr(1) ;

            static Cache g_cache ;

            auto stored = g_cache.fetch(filepath) ;
            if ( stored ) return stored ;
            else {
                std::ifstream strm(root_folder_ + filepath) ;

                if ( strm ) {
                    string contents((istreambuf_iterator<char>(strm)),
                            istreambuf_iterator<char>());
                    auto compiled = parseString(contents) ;
                    g_cache.add(filepath, compiled) ;
                    return compiled ;
                }
                else return nullptr ;
            }
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


private:
    uint idx_ ;
    const Partials &partials_ ;
    const string &root_folder_ ;
};


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
            else if ( idx_ < nc ) raw.push_back(src[idx_++]) ;
        }
        else if ( idx_ < nc ) raw.push_back(src[idx_++]) ;
    }

    return false ;
}

SectionNode::Ptr Parser::parseString(const string &src) {

    SectionNode::Ptr root(new SectionNode("$root")) ;

    deque<SectionNode::Ptr> stack ;

    stack.push_back(root) ;

    while (!stack.empty()) {
        SectionNode::Ptr parent = stack.back() ;

        string raw ;
        Tag tag ;
        bool res = nextTag(src, raw, tag)  ;

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
            auto p = partials_.find(tag.name_) ;
            if ( p != partials_.end() ) {

                Parser parser(partials_, root_folder_) ;
                auto ast = parser.parse(p->second) ;
                if ( ast ) {
                    for( auto &c: ast->children_ )
                        parent->children_.push_back(c) ;
                }
            }
        }


        if ( !res ) stack.pop_back() ;

    }

    return root ;
}

}

string TemplateRenderer::render(const string &src, const Variant &ctx, const Partials &partials) {
    using namespace mustache ;

    Parser parser(partials, root_folder_) ;

    string res ;
    SectionNode::Ptr ast = parser.parse(src) ;

    if ( ast ) {
        Variant rc(Variant::Object{{"$root", ctx}}) ;
        ast->eval(rc, res) ;
    }

    return res ;

}





}
