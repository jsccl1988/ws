#ifndef __TEMPLATE_AST_HPP__
#define __TEMPLATE_AST_HPP__

#include <memory>
#include <deque>
#include <vector>
#include <boost/regex.hpp>
#include <boost/optional.hpp>

#include <wspp/util/variant.hpp>

#include "context.hpp"

using wspp::util::Variant ;

class TemplateRenderer ;

namespace detail {



enum WhiteSpace { TrimNone = 0, TrimLeft = 1, TrimRight = 2, TrimBoth = TrimLeft | TrimRight } ;

class ExpressionNode ;

typedef std::shared_ptr<ExpressionNode> ExpressionNodePtr ;

class ExpressionNode {
public:
    ExpressionNode() {}

    virtual Variant eval(TemplateEvalContext &ctx) = 0 ;
};

class LiteralNode: public ExpressionNode {
public:

    LiteralNode(const Variant &v): val_(v) {}

    virtual Variant eval(TemplateEvalContext &ctx) { return val_ ; }

    Variant val_;
};

class ExpressionList {
public:
    ExpressionList() {}

    void append(ExpressionNodePtr node) { children_.push_back(node) ; }
    void prepend(ExpressionNodePtr node) { children_.push_front(node) ; }

    const std::deque<ExpressionNodePtr> &children() const { return children_ ; }

private:
    std::deque<ExpressionNodePtr> children_ ;
};


typedef std::shared_ptr<ExpressionList> ExpressionListPtr ;

class KeyValNode {
public:
    KeyValNode(const std::string &key, ExpressionNodePtr val): key_(key), val_(val) {}

    std::string key_;
    ExpressionNodePtr val_ ;
};

typedef std::shared_ptr<KeyValNode> KeyValNodePtr ;

class KeyValList {
public:
    KeyValList() {}

    void append(KeyValNodePtr node) { children_.push_back(node) ; }
    void prepend(KeyValNodePtr node) { children_.push_front(node) ; }

    const std::deque<KeyValNodePtr> &children() const { return children_ ; }

private:
    std::deque<KeyValNodePtr> children_ ;
};

typedef std::shared_ptr<KeyValList> KeyValListPtr ;

class IdentifierList {
public:
    IdentifierList() {}

    void append(const std::string &item) { children_.push_back(item) ; }
    void prepend(const std::string &item) { children_.push_front(item) ; }

    const std::deque<std::string> &children() const { return children_ ; }

private:
    std::deque<std::string> children_ ;
};

typedef std::shared_ptr<IdentifierList> IdentifierListPtr ;


class ValueNode: public ExpressionNode {
public:

    ValueNode(ExpressionNodePtr &l): val_(l) {}

    Variant eval(TemplateEvalContext &ctx) { return val_->eval(ctx) ; }

    ExpressionNodePtr val_ ;
};

class IdentifierNode: public ExpressionNode {
public:
    IdentifierNode(const std::string name): name_(name) {}

    Variant eval(TemplateEvalContext &ctx) ;

private:
    std::string name_ ;
};

class ArrayNode: public ExpressionNode {
public:
    ArrayNode(ExpressionListPtr elements): elements_(elements) {}

    Variant eval(TemplateEvalContext &ctx) ;

private:
    ExpressionListPtr elements_ ;
};

class DictionaryNode: public ExpressionNode {
public:
    DictionaryNode(KeyValListPtr elements): elements_(elements) {}

    Variant eval(TemplateEvalContext &ctx) ;

private:
    KeyValListPtr elements_ ;
};

class SubscriptIndexingNode: public ExpressionNode {
public:
    SubscriptIndexingNode(ExpressionNodePtr array, ExpressionNodePtr index): array_(array), index_(index) {}

    Variant eval(TemplateEvalContext &ctx) ;

private:
    ExpressionNodePtr array_, index_ ;
};

class AttributeIndexingNode: public ExpressionNode {
public:
    AttributeIndexingNode(ExpressionNodePtr dict, const std::string &key): dict_(dict), key_(key) {}

    Variant eval(TemplateEvalContext &ctx) ;

private:
    ExpressionNodePtr dict_ ;
    std::string key_ ;
};

class BinaryOperator: public ExpressionNode {
public:
    BinaryOperator(int op, ExpressionNodePtr lhs, ExpressionNodePtr rhs): op_(op), lhs_(lhs), rhs_(rhs) {}

    Variant eval(TemplateEvalContext &ctx) ;

private:
    int op_ ;
    ExpressionNodePtr lhs_, rhs_ ;
};


class BooleanOperator: public ExpressionNode {
public:
    enum Type { And, Or, Not } ;

    BooleanOperator(Type op, ExpressionNodePtr lhs, ExpressionNodePtr rhs): op_(op), lhs_(lhs), rhs_(rhs) {}

    Variant eval(TemplateEvalContext &ctx) ;
private:
    Type op_ ;
    ExpressionNodePtr lhs_, rhs_ ;
};

class UnaryOperator: public ExpressionNode {
public:

    UnaryOperator(char op, ExpressionNodePtr rhs): op_(op), rhs_(rhs) {}

    Variant eval(TemplateEvalContext &ctx) ;
private:
    char op_ ;
    ExpressionNodePtr rhs_ ;
};


class ComparisonPredicate: public ExpressionNode {
public:
    enum Type { Equal, NotEqual, Less, Greater, LessOrEqual, GreaterOrEqual } ;

    ComparisonPredicate(Type op, ExpressionNodePtr lhs, ExpressionNodePtr rhs): op_(op), lhs_(lhs), rhs_(rhs) {}


    Variant eval(TemplateEvalContext &ctx) ;

private:
    Type op_ ;
    ExpressionNodePtr lhs_, rhs_ ;
};

class TernaryExpressionNode: public ExpressionNode {
public:
    TernaryExpressionNode(ExpressionNodePtr cond, ExpressionNodePtr t, ExpressionNodePtr f): condition_(cond), positive_(t), negative_(f) {}

    Variant eval(TemplateEvalContext &ctx) ;
private:
    ExpressionNodePtr condition_, positive_, negative_ ;
};

class FunctionArg {
public:
    FunctionArg(ExpressionNodePtr val, const std::string &name = std::string()): name_(name), val_(val) {}

    ExpressionNodePtr val_ ;
    std::string name_ ;
};

typedef std::shared_ptr<FunctionArg> FunctionArgPtr ;


class FunctionArguments {
public:
    FunctionArguments() {}

    void eval(Variant &args, TemplateEvalContext &ctx, const boost::optional<Variant> &extra) const ;

    void append(FunctionArgPtr node) { children_.push_back(node) ; }
    void prepend(FunctionArgPtr node) { children_.push_front(node) ; }

    const std::deque<FunctionArgPtr> &children() const { return children_ ; }

private:
    std::deque<FunctionArgPtr> children_ ;

};

typedef std::shared_ptr<FunctionArguments> FunctionArgumentsPtr ;


class FilterNode {
public:
    FilterNode(const std::string &name, FunctionArgumentsPtr args): name_(name), args_(args) {}
    FilterNode(const std::string &name): name_(name) {}

    Variant eval(const Variant &target, TemplateEvalContext &ctx) ;

   // void evalArgs(const Variant &target, Variant &args, TemplateEvalContext &ctx) const ;
    static Variant dispatch(const std::string &, const Variant &args) ;

private:
    std::string name_ ;
    FunctionArgumentsPtr args_ ;
};

typedef std::shared_ptr<FilterNode> FilterNodePtr ;

class InvokeFilterNode: public ExpressionNode {
public:
    InvokeFilterNode(ExpressionNodePtr target, FilterNodePtr filter): target_(target), filter_(filter) {}

    Variant eval(TemplateEvalContext &ctx) ;


private:
    ExpressionNodePtr target_ ;
    FilterNodePtr filter_ ;
};

class InvokeFunctionNode: public ExpressionNode {
public:
    InvokeFunctionNode(ExpressionNodePtr callable, FunctionArgumentsPtr args): callable_(callable), args_(args) {}

    Variant eval(TemplateEvalContext &ctx) ;


private:
    ExpressionNodePtr callable_ ;
    FunctionArgumentsPtr args_ ;
};

class InvokeGlobalFunctionNode: public ExpressionNode {
public:
    InvokeGlobalFunctionNode(const std::string &name, FunctionArgumentsPtr args): name_(name), args_(args) {}

    Variant eval(TemplateEvalContext &ctx) ;


private:
    std::string name_ ;
    FunctionArgumentsPtr args_ ;
};

class DocumentNode ;

class ContentNode {
public:
    ContentNode(WhiteSpace ws = WhiteSpace::TrimNone): ws_(ws) {}
    virtual ~ContentNode() {}
    // evalute a node using input context and put result in res
    virtual void eval(TemplateEvalContext &ctx, std::string &res) const = 0 ;

    void trim(const std::string &src, std::string &out) const;

    static std::string escape(const std::string &src) ;

    const DocumentNode *root() const {
        const ContentNode *node = this ;
        while ( node->parent_ ) {
            node = node->parent_ ;
        }

        return reinterpret_cast<const DocumentNode *>(node) ;
    }

    void setWhiteSpace(WhiteSpace ws) {
        ws_ = ws ;
    }

    WhiteSpace ws_ ;

    ContentNode *parent_ = nullptr ;
};

typedef std::shared_ptr<ContentNode> ContentNodePtr ;

class ContainerNode: public ContentNode {
public:

    void addChild(ContentNodePtr child) {
        children_.push_back(child) ;
        child->parent_ = this ;
    }

    virtual std::string endContainerTag() const { return {} ; }
    std::vector<ContentNodePtr> children_ ;
};

typedef std::shared_ptr<ContainerNode> ContainerNodePtr ;

class ForLoopBlockNode: public ContainerNode {
public:

    ForLoopBlockNode(IdentifierListPtr ids, ExpressionNodePtr target): ids_(std::move(ids->children())), target_(target) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override ;

    virtual std::string endContainerTag() const { return "endfor" ; }

    void startElseBlock() {
        else_child_start_ = children_.size() ;
    }

    int else_child_start_ = -1 ;

    std::deque<std::string> ids_ ;
    ExpressionNodePtr target_ ;
};


class NamedBlockNode: public ContainerNode {
public:

    NamedBlockNode(const std::string &name): name_(name) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override ;

    virtual std::string endContainerTag() const { return "endblock" ; }

    std::string name_ ;
};

typedef std::shared_ptr<NamedBlockNode> NamedBlockNodePtr ;

class ExtensionBlockNode: public ContainerNode {
public:

    ExtensionBlockNode(ExpressionNodePtr src): parent_resource_(src) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override ;

    virtual std::string endContainerTag() const { return "endextends" ; }

    ExpressionNodePtr parent_resource_ ;
};


class IfBlockNode: public ContainerNode {
public:

    IfBlockNode(ExpressionNodePtr target) { addBlock(target) ; }

    void eval(TemplateEvalContext &ctx, std::string &res) const override ;

    virtual std::string endContainerTag() const { return "endif" ; }

    void addBlock(ExpressionNodePtr ptr) {
        if ( !blocks_.empty() ) {
            blocks_.back().cstop_ = children_.size() ;
        }
        int cstart = blocks_.empty() ? 0 : blocks_.back().cstop_ ;
        blocks_.push_back({cstart, -1, ptr}) ;
    }

    struct Block {
        int cstart_, cstop_ ;
        ExpressionNodePtr condition_ ;
    } ;

    std::vector<Block> blocks_ ;

};

class AssignmentBlockNode: public ContainerNode {
public:

    AssignmentBlockNode(const std::string &id, ExpressionNodePtr val): id_(id), val_(val) { }

    void eval(TemplateEvalContext &ctx, std::string &res) const override ;

    virtual std::string endContainerTag() const { return "endset" ; }

    ExpressionNodePtr val_ ;
    std::string id_ ;

};

class FilterBlockNode: public ContainerNode {
public:

    FilterBlockNode(FilterNodePtr filter): filter_(filter) { }

    void eval(TemplateEvalContext &ctx, std::string &res) const override ;

    virtual std::string endContainerTag() const { return "endfilter" ; }

    FilterNodePtr filter_ ;
};

class MacroBlockNode: public ContainerNode {
public:

    MacroBlockNode(const std::string &name, IdentifierListPtr args): name_(name), args_(std::move(args->children())) { }
    MacroBlockNode(const std::string &name): name_(name) { }

    void eval(TemplateEvalContext &ctx, std::string &res) const override ;

    void mapArguments(const Variant &args, Variant::Object &ctx, Variant::Array &arg_list) ;

    virtual std::string endContainerTag() const { return "endmacro" ; }

    std::string name_ ;
    std::deque<std::string> args_ ;

};

class ImportBlockNode: public ContainerNode {
public:

    ImportBlockNode(ExpressionNodePtr source, const std::string &ns): source_(source), ns_(ns) { }

    void eval(TemplateEvalContext &ctx, std::string &res) const override ;

    void addMacroClosureToContext(TemplateEvalContext &ctx, MacroBlockNode &n) const;

    virtual std::string endContainerTag() const { return "endimport" ; }

    std::string ns_ ;
    ExpressionNodePtr source_ ;
};

class RawTextNode: public ContentNode {
public:
    RawTextNode(const std::string &text): text_(text) {}

    void eval(TemplateEvalContext &, std::string &res) const override {
        res.append(text_) ;
    }

    std::string text_ ;
};

class SubTextNode: public ContentNode {
public:
    SubTextNode(ExpressionNodePtr expr, WhiteSpace ws = WhiteSpace::TrimNone): expr_(expr), ContentNode(ws) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override {
        std::string contents = expr_->eval(ctx).toString() ;
        trim(contents, res) ;
    }

    ExpressionNodePtr expr_ ;
};


class DocumentNode: public ContainerNode {
public:

    DocumentNode(TemplateRenderer &rdr): renderer_(rdr) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override {
        for( auto &&e: children_ )
            e->eval(ctx, res) ;
    }

    std::map<std::string, detail::ContentNodePtr> macro_blocks_ ;
    TemplateRenderer &renderer_ ;
};

typedef std::shared_ptr<DocumentNode> DocumentNodePtr ;

} // namespace template_ast

#endif
