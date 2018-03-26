#ifndef __TEMPLATE_AST_HPP__
#define __TEMPLATE_AST_HPP__

#include <memory>
#include <deque>
#include <vector>
#include <boost/regex.hpp>

#include <wspp/util/variant.hpp>

using wspp::util::Variant ;

namespace ast {

struct TemplateEvalContext {

    Variant::Object &push() {
        if ( stack_.empty() )
            stack_.push(Variant::Object()) ;
        else
            stack_.push(stack_.top()) ;
        return top() ;
    }

    void pop() {
        if ( !stack_.empty())
            stack_.pop() ;
    }

    Variant::Object &top() {
        return stack_.top() ;
    }

    std::stack<Variant::Object> stack_ ;
};

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

class UnaryPredicate: public ExpressionNode {
public:

    UnaryPredicate(ExpressionNodePtr exp): exp_(exp) {}

    Variant eval(TemplateEvalContext &ctx) ;

    ExpressionNodePtr exp_ ;
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


class FilterNode {
public:
    FilterNode(const std::string &name, ExpressionListPtr args): name_(name), args_(args) {}
    FilterNode(const std::string &name): name_(name) {}

    Variant eval(const Variant &target, TemplateEvalContext &ctx) ;

    void evalArgs(Variant::Array &args, TemplateEvalContext &ctx) const ;
    static Variant dispatch(const std::string &, const Variant::Array &args) ;

private:
    std::string name_ ;
    ExpressionListPtr args_ ;
};

typedef std::shared_ptr<FilterNode> FilterNodePtr ;

class ApplyFilterNode: public ExpressionNode {
public:
    ApplyFilterNode(ExpressionNodePtr target, FilterNodePtr filter): target_(target), filter_(filter) {}

    Variant eval(TemplateEvalContext &ctx) ;


private:
    ExpressionNodePtr target_ ;
    FilterNodePtr filter_ ;
};

class ContentNode {
public:
    // evalute a node using input context and put result in res
    virtual void eval(TemplateEvalContext &ctx, std::string &res) const = 0 ;



    static std::string escape(const std::string &src) ;
};

typedef std::shared_ptr<ContentNode> ContentNodePtr ;

class ContainerNode: public ContentNode {
public:

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
    SubTextNode(ExpressionNodePtr expr): expr_(expr) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override {
        res.append(expr_->eval(ctx).toString()) ;
    }

    ExpressionNodePtr expr_ ;
};


class DocumentNode: public ContainerNode {
public:

    void eval(TemplateEvalContext &ctx, std::string &res) const override {
        for( auto &&e: children_ )
            e->eval(ctx, res) ;
    }

};


} // namespace template_ast

#endif
