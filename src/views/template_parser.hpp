#ifndef __TEMPLATE_PARSER_HPP__
#define __TEMPLATE_PARSER_HPP__

#include <string>
#include <memory>

#include <wspp/views/renderer.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using wspp::util::Variant ;

namespace wspp { namespace web {
namespace detail {

class Parser ;

struct Tag {
    enum Type { None, EscapedSubstitution, RawSubstitutionAmpersand, RawSubstitutionCurlyBracket,
                Comment, Block, Extension, SectionBegin, SectionEnd, InvertedSectionBegin, Partial } ;

    Tag(): type_(None)  {}
    std::string name_ ;
    Type type_ ;
};

// parser AST nodes

struct Node {

    typedef std::shared_ptr<Node> Ptr ;

    enum Type { RawText, Section, Comment, Substitution, Partial, Extension, Block, Helper } ;

    virtual Type type() const = 0;

    // evalute a node using input context and put result in res
    virtual void eval(ContextStack &ctx, std::string &res) const = 0 ;

    static std::string escape(const std::string &src) ;
};

struct RawTextNode: public Node {

    typedef std::shared_ptr<RawTextNode> Ptr ;

    RawTextNode(const std::string &text): text_(text) {}

    // raw text so just copy contents to output buffer
    void eval(ContextStack &ctx, std::string &res) const override {
        res.append(text_) ;
    }

    Type type() const override { return RawText ; }

    std::string text_ ;
};

struct BlockNode ;

struct ContainerNode: public Node {
    typedef std::shared_ptr<ContainerNode> Ptr ;

    ContainerNode(const std::string &name): name_(name) {}

    std::string name_ ;
    std::vector<Node::Ptr> children_ ;
    std::map<std::string, Node::Ptr> blocks_ ;
};

// section
struct SectionNode: public ContainerNode {

    typedef std::shared_ptr<SectionNode> Ptr ;

    SectionNode(const std::string &name, bool is_inverted = false): ContainerNode(name), is_inverted_(is_inverted) {}

    Type type() const override { return Section ; }

    void eval(ContextStack &ctx, std::string &res) const override {
        const Variant &val = ctx.find(name_) ;
        if ( !is_inverted_ ) {
            if ( val.isFalse() ) return ; // either does not exist or set to explicitly set to Null, false, empty array
            else if ( val.isArray() ) { // this is an array so iterate children nodes passing the current array element on to of the stack
                for( const Variant &v: val ) {
                    ctx.push(v) ;
                    for( auto &c: children_) {
                        c->eval(ctx, res) ;
                    }
                    ctx.pop() ;
                }
            }
            else if ( val.isObject() ) { // push variable to context and evaluate children
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
        else { // inverted section
            if ( val.isFalse() ) { // evalute only if value is false
                for( auto &c: children_) {
                    c->eval(ctx, res)  ;
                }
            }
        }
    }

    bool is_inverted_ ;
};

struct BlockNode: public ContainerNode {

    typedef std::shared_ptr<BlockNode> Ptr ;

    BlockNode(const std::string &name): ContainerNode(name) {}

    Type type() const override { return Block ; }

    // A block node just evalutes its children
    void eval(ContextStack &ctx, std::string &res) const override {
        for( auto &c: children_) {
            c->eval(ctx, res) ;
        }
    }
};

struct SubstitutionNode: public Node {

    typedef std::shared_ptr<SubstitutionNode> Ptr ;

    SubstitutionNode(const std::string &var, bool is_escaped): var_(var), is_escaped_(is_escaped) {}

    // substitute variables inline
    void eval(ContextStack &ctx, std::string &res) const override {

        std::string sub ;
        const Variant &val = ( var_ == "." ) ? ctx.top() : ctx.find(var_) ;

        if ( val.isNull() ) return ;
        else if ( val.isPrimitive() ) sub = val.toString() ;
        else return ;

        res.append((is_escaped_ ) ? escape(sub) : sub ) ;
    }

    Type type() const override { return Substitution ; }

    std::string var_ ;
    bool is_escaped_ ;
};

struct ParseContext {
    ParseContext(const std::shared_ptr<wspp::web::TemplateLoader> &loader,
                 const std::map<std::string, wspp::web::TemplateRenderer::BlockHelper> &block_helpers,
                 const std::map<std::string, wspp::web::TemplateRenderer::ValueHelper> &value_helpers,
                 bool caching):
        loader_(loader), block_helpers_(block_helpers), value_helpers_(value_helpers), caching_(caching) {}

    std::shared_ptr<wspp::web::TemplateLoader> loader_ ;
    const std::map<std::string, wspp::web::TemplateRenderer::BlockHelper> &block_helpers_ ;
    const std::map<std::string, wspp::web::TemplateRenderer::ValueHelper> &value_helpers_ ;
    bool caching_ ;
} ;

class Parser {
public:

    Parser(const ParseContext &ctx): ctx_(ctx), idx_(0) {
    }

    SectionNode::Ptr parse(const std::string &key);
    SectionNode::Ptr parseString(const std::string &src) ;

    struct Arg {
        Arg(const std::string &key, const std::string &val, bool is_literal): key_(key), val_(val), is_literal_(is_literal) {}

        std::string key_ ;
        std::string val_ ;
        bool is_literal_ ;
    };

    static Variant::Object getDictionaryArgs(const std::vector<Arg> &args, ContextStack &stack);
    static Variant::Array getParams(const std::vector<Arg> &args, ContextStack &ctx);

private:

    class Cache {
    public:

        typedef SectionNode::Ptr Entry ;

        void add(const std::string &key, SectionNode::Ptr &val) {
            boost::mutex::scoped_lock lock(guard_);
            compiled_.insert({key, val}) ;
        }

        Entry fetch(const std::string &key) {
            boost::mutex::scoped_lock lock(guard_);
            auto it = compiled_.find(key) ;
            if ( it != compiled_.end() ) return it->second ;
            else return nullptr ;
        }

    private:
        std::map<std::string, Entry> compiled_ ;
        boost::mutex guard_ ;

    };

    struct Position {
        Position(const std::string &src): cursor_(src.begin()), end_(src.end()){}

        operator bool () const { return cursor_ != end_ ; }
        char operator * () const { return *cursor_ ; }
        Position& operator++() { advance(); return *this ; }
        Position operator++(int) {
            Position p(*this) ;
            advance() ;
            return p ;
        }

        Position& operator--() { backtrack(); return *this ; }

        void advance() {
            // skip new line characters
            column_++ ;

            if ( cursor_ != end_ && *cursor_ == '\n' ) {
                column_ = 1 ; line_ ++ ;
            }

            cursor_ ++ ;
        }

        void backtrack() {
            // skip new line characters
            cursor_ -- ;
            column_-- ;

            if ( cursor_ != end_ && *cursor_ == '\n' ) {
                column_ = 1 ; line_ -- ;
            }


        }

        std::string::const_iterator cursor_, end_ ;
        uint column_ = 1;
        uint line_ = 1;
    } ;

    bool expect(Position &, char c) ;
    std::string eatTag(Position &) ;
    void parseTag(Position &, Tag &tag) ;
    bool nextTag(Position &, std::string &raw, Tag &tag, std::string::const_iterator &cursor) ;
    bool parseComplexTag(const std::string &tag, std::string &name, std::vector<Arg> &args) ;
    bool parseSimpleTag(const std::string &tag, std::string &name) ;

private:
    friend class PartialNode ;
    friend class ExtensionNode ;



    uint idx_ ;

    ParseContext ctx_ ;
  //  Position cursor_ ;
   // bool caching_ ;
};


struct PartialNode: public Node {

    typedef std::shared_ptr<PartialNode> Ptr ;

    // inherit parent parameters
    PartialNode(const std::string &name, const std::vector<Parser::Arg> &args, const ParseContext &context):
        key_(name), args_(args), context_(context) {
    }

    void eval(ContextStack &ctx, std::string &res) const override {

        std::string key ;

        const Variant &v = ctx.find(key_) ;
        if ( v.isPrimitive() )
            key = v.toString() ;

        if ( key.empty() ) key = key_ ;

        Parser parser(context_) ;
        auto ast = parser.parse(key) ;
        if ( ast ) {
            Variant args(Parser::getDictionaryArgs(args_, ctx)) ;
            ctx.push(args) ;
            ast->eval(ctx, res) ;
            ctx.pop() ;
        }
    }

    Type type() const override { return Partial ; }

    std::string key_ ;
    std::vector<Parser::Arg> args_ ;
    const ParseContext &context_ ;
};

struct BlockHelperNode: public Node {


    typedef std::shared_ptr<BlockHelperNode> Ptr ;

    // inherit parent parameters
    BlockHelperNode(const std::string &name, const std::vector<Parser::Arg> &args, TemplateRenderer::BlockHelper helper): key_(name), args_(args), helper_(helper) {}

    void eval(ContextStack &ctx, std::string &res) const override {

        Variant args(Parser::getDictionaryArgs(args_, ctx)) ;
        ctx.push(args) ;
        Variant::Array params = Parser::getParams(args_, ctx) ;
        std::string part = helper_(content_, ctx, params) ;
        ctx.pop() ;
        res.append(part) ;

    }

    void setContent(const std::string &content) {
        content_ = content ;
    }

    Type type() const override { return Helper ; }

    std::string key_, content_ ;
    TemplateRenderer::BlockHelper helper_ ;
    std::vector<Parser::Arg> args_ ;

};

struct ValueHelperNode: public Node {

    typedef std::shared_ptr<ValueHelperNode> Ptr ;

    // inherit parent parameters
    ValueHelperNode(const std::string &name, const std::vector<Parser::Arg> &args, TemplateRenderer::ValueHelper helper, bool escape):
        key_(name), args_(args), helper_(helper), escape_(escape) {}

    void eval(ContextStack &ctx, std::string &res) const override {

        Variant args(Parser::getDictionaryArgs(args_, ctx)) ;
        ctx.push(args) ;
        Variant::Array params = Parser::getParams(args_, ctx) ;
        std::pair<bool, std::string> part = helper_(ctx, params) ;
        ctx.pop() ;

        if ( escape_ && !part.first ) res.append(escape(part.second)) ;
        else res.append(part.second) ;
    }

    Type type() const override { return Helper ; }

    std::string key_ ;
    TemplateRenderer::ValueHelper helper_ ;
    std::vector<Parser::Arg> args_ ;
    bool escape_ ;

};

struct ExtensionNode: public ContainerNode {

    typedef std::shared_ptr<ExtensionNode> Ptr ;

    // inherit parent parameters
    ExtensionNode(const std::string &name, const std::vector<Parser::Arg> &args, const ParseContext &context): context_(context), ContainerNode(name) {
    }

    void eval(ContextStack &ctx, std::string &res) const override {

        // load base node

        Parser parser(context_) ;
        auto ast = parser.parse(name_) ;
        if ( ast ) {
            // replace parent blocks with child blocks

            Variant args(Parser::getDictionaryArgs(args_, ctx)) ;
            ctx.push(args) ;

            for( auto &c: ast->children_ ) {
                if ( c->type() == Node::Block ) {
                    BlockNode::Ptr block = std::dynamic_pointer_cast<BlockNode>(c) ;
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
    const ParseContext &context_ ;
    std::vector<Parser::Arg> args_ ;

};

} // namespace detail
} // namespace web
} // namespace wspp
#endif
