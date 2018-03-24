#ifndef __TEMPLATE_AST_HPP__
#define __TEMPLATE_AST_HPP__

#include <memory>
#include <deque>
#include <vector>
#include <boost/regex.hpp>

namespace ast {

struct TemplateEvalContext {};

class ExpressionNode ;

typedef std::shared_ptr<ExpressionNode> ExpressionNodePtr ;

struct Literal {
public:
    enum Type { String, Number, Boolean, Null } ;

    Literal(): type_(Null) {}
    Literal(const std::string &val, bool auto_conv = true) ;
    Literal(const double val): type_(Number), number_val_(val) {}
    Literal(const bool val): type_(Boolean), boolean_val_(val) {}

    bool isNull() const { return type_ == Null ; }
    bool toBoolean() const ;
    std::string toString() const ;
    double toNumber() const ;

    Type type_ ;
    std::string string_val_ ;
    double number_val_ ;
    bool boolean_val_ ;
};


class ExpressionNode {
public:

    ExpressionNode() {}

    virtual Literal eval(TemplateEvalContext &ctx) { return false ; }
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


class LiteralExpressionNode: public ExpressionNode {
public:

    LiteralExpressionNode(const Literal &l): val_(l) {}

    Literal eval(TemplateEvalContext &ctx) { return val_ ; }

    Literal val_ ;
};

class Attribute: public ExpressionNode {
public:
    Attribute(const std::string name): name_(name) {}

    Literal eval(TemplateEvalContext &ctx) ;

private:
    std::string name_ ;
};


class Function: public ExpressionNode {
public:
    Function(const std::string &name): name_(name) {}
    Function(const std::string &name, const std::deque<ExpressionNodePtr> &args): name_(name), args_(args) {}

    Literal eval(TemplateEvalContext &ctx) ;

private:
    std::string name_ ;
    std::deque<ExpressionNodePtr> args_ ;
};


class BinaryOperator: public ExpressionNode {
public:
    BinaryOperator(int op, ExpressionNodePtr lhs, ExpressionNodePtr rhs): op_(op), lhs_(lhs), rhs_(rhs) {}

    Literal eval(TemplateEvalContext &ctx) ;

private:
    int op_ ;
    ExpressionNodePtr lhs_, rhs_ ;
};


class BooleanOperator: public ExpressionNode {
public:
    enum Type { And, Or, Not } ;

    BooleanOperator(Type op, ExpressionNodePtr lhs, ExpressionNodePtr rhs): op_(op), lhs_(lhs), rhs_(rhs) {}

    Literal eval(TemplateEvalContext &ctx) ;
private:
    Type op_ ;
    ExpressionNodePtr lhs_, rhs_ ;
};

class UnaryPredicate: public ExpressionNode {
public:

    UnaryPredicate(ExpressionNodePtr exp): exp_(exp) {}

    Literal eval(TemplateEvalContext &ctx) ;

    ExpressionNodePtr exp_ ;
};

class ComparisonPredicate: public ExpressionNode {
public:
    enum Type { Equal, NotEqual, Less, Greater, LessOrEqual, GreaterOrEqual } ;

    ComparisonPredicate(Type op, ExpressionNodePtr lhs, ExpressionNodePtr rhs): op_(op), lhs_(lhs), rhs_(rhs) {}


    Literal eval(TemplateEvalContext &ctx) ;

private:
    Type op_ ;
    ExpressionNodePtr lhs_, rhs_ ;
};

class LikeTextPredicate: public ExpressionNode {
public:

    LikeTextPredicate(ExpressionNodePtr op, const std::string &pattern_, bool is_pos) ;

    Literal eval(TemplateEvalContext &ctx) ;
private:
    ExpressionNodePtr exp_ ;
    boost::regex pattern_ ;
    bool is_pos_ ;
};

class ListPredicate: public ExpressionNode {
public:

    ListPredicate(const std::string &identifier, const std::deque<ExpressionNodePtr> &op, bool is_pos) ;

    Literal eval(TemplateEvalContext &ctx) ;

private:
    std::vector<ExpressionNodePtr> children_ ;
    std::string id_ ;
    std::vector<std::string> lvals_ ;
    bool is_pos_ ;

};

} // namespace template_ast

#endif
