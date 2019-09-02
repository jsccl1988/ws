#ifndef __TWIG_AST_HPP__
#define __TWIG_AST_HPP__

#include <memory>
#include <deque>
#include <vector>
#include <boost/regex.hpp>
#include <boost/optional.hpp>

#include <wspp/util/variant.hpp>

#include <wspp/twig/context.hpp>

using wspp::util::Variant;


namespace wspp {
namespace twig {

class TemplateRenderer;
namespace detail {

class ExpressionNode;
typedef std::shared_ptr<ExpressionNode> ExpressionNodePtr;

class ExpressionNode {
public:
    ExpressionNode() {}

    virtual Variant eval(TemplateEvalContext &ctx) = 0;
};

class LiteralNode: public ExpressionNode {
public:
    LiteralNode(const Variant &v): val_(v) {}

    virtual Variant eval(TemplateEvalContext &ctx) { return val_; }

    Variant val_;
};

using key_val_t = std::pair<std::string, ExpressionNodePtr>;
using key_val_list_t = std::deque<key_val_t>;
using identifier_list_t = std::deque<std::string>;
using key_alias_t = std::pair<std::string, std::string>;
using key_alias_list_t = std::deque<key_alias_t>;

class ValueNode: public ExpressionNode {
public:
    ValueNode(ExpressionNodePtr &l): val_(l) {}

    Variant eval(TemplateEvalContext &ctx) { return val_->eval(ctx); }

    ExpressionNodePtr val_;
};

class IdentifierNode: public ExpressionNode {
public:
    IdentifierNode(const std::string name): name_(name) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    std::string name_;
};

class ArrayNode: public ExpressionNode {
public:
    ArrayNode() = default;
    ArrayNode(const std::deque<ExpressionNodePtr> &&elements): elements_(elements) {}

    Variant eval(TemplateEvalContext &ctx);

private:

    std::deque<ExpressionNodePtr> elements_;
};

class ContainmentNode: public ExpressionNode {
public:
    ContainmentNode(ExpressionNodePtr lhs, ExpressionNodePtr rhs, bool positive):
        lhs_(lhs), rhs_(rhs), positive_(positive) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    ExpressionNodePtr lhs_, rhs_;
    bool positive_;
};

class MatchesNode: public ExpressionNode {
public:
    MatchesNode(ExpressionNodePtr lhs, const std::string &rx, bool positive);

    Variant eval(TemplateEvalContext &ctx);

private:
    ExpressionNodePtr lhs_;
    boost::regex rx_;
    bool positive_;
};

class DictionaryNode: public ExpressionNode {
public:
    DictionaryNode() = default;
    DictionaryNode(const key_val_list_t && elements): elements_(elements) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    key_val_list_t elements_;
};

class SubscriptIndexingNode: public ExpressionNode {
public:
    SubscriptIndexingNode(ExpressionNodePtr array, ExpressionNodePtr index): array_(array), index_(index) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    ExpressionNodePtr array_, index_;
};

class AttributeIndexingNode: public ExpressionNode {
public:
    AttributeIndexingNode(ExpressionNodePtr dict, const std::string &key): dict_(dict), key_(key) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    ExpressionNodePtr dict_;
    std::string key_;
};

class BinaryOperator: public ExpressionNode {
public:
    BinaryOperator(int op, ExpressionNodePtr lhs, ExpressionNodePtr rhs): op_(op), lhs_(lhs), rhs_(rhs) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    int op_;
    ExpressionNodePtr lhs_, rhs_;
};

class BooleanOperator: public ExpressionNode {
public:
    enum Type { And, Or, Not };

    BooleanOperator(Type op, ExpressionNodePtr lhs, ExpressionNodePtr rhs): op_(op), lhs_(lhs), rhs_(rhs) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    Type op_;
    ExpressionNodePtr lhs_, rhs_;
};

class UnaryOperator: public ExpressionNode {
public:
    UnaryOperator(char op, ExpressionNodePtr rhs): op_(op), rhs_(rhs) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    char op_;
    ExpressionNodePtr rhs_;
};


class ComparisonPredicate: public ExpressionNode {
public:
    enum Type { Equal, NotEqual, Less, Greater, LessOrEqual, GreaterOrEqual };

    ComparisonPredicate(Type op, ExpressionNodePtr lhs, ExpressionNodePtr rhs): op_(op), lhs_(lhs), rhs_(rhs) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    Type op_;
    ExpressionNodePtr lhs_, rhs_;
};

class TernaryExpressionNode: public ExpressionNode {
public:
    TernaryExpressionNode(ExpressionNodePtr cond, ExpressionNodePtr t, ExpressionNodePtr f):
        condition_(cond),
        positive_(t), negative_(f) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    ExpressionNodePtr condition_, positive_, negative_;
};

class ContextNode: public ExpressionNode {
public:
    ContextNode() {}

    Variant eval(TemplateEvalContext &ctx) {
        return ctx.data();
    }
};

class InvokeFilterNode: public ExpressionNode {
public:
    InvokeFilterNode(ExpressionNodePtr target, const std::string &name, key_val_list_t &&args ={}): target_(target), name_(name),
        args_(args) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    ExpressionNodePtr target_;
    std::string name_;
    key_val_list_t args_;
};

class InvokeTestNode: public ExpressionNode {
public:
    InvokeTestNode(ExpressionNodePtr target, const std::string &name,
                   key_val_list_t &&args, bool positive):
        target_(target), name_(name), args_(args), positive_(positive) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    ExpressionNodePtr target_;
    std::string name_;
    key_val_list_t args_;
    bool positive_;
};


class InvokeFunctionNode: public ExpressionNode {
public:
    InvokeFunctionNode(ExpressionNodePtr callable, key_val_list_t &&args = {}): callable_(callable), args_(args) {}

    Variant eval(TemplateEvalContext &ctx);

private:
    ExpressionNodePtr callable_;
    key_val_list_t args_;
};

class DocumentNode;
class ContentNode {
public:
    ContentNode() {}
    virtual ~ContentNode() {}
    // evalute a node using input context and put result in res
    virtual void eval(TemplateEvalContext &ctx, std::string &res) const = 0;

    const DocumentNode *root() const {
        const ContentNode *node = this;
        while ( node->parent_ ) {
            node = node->parent_;
        }

        return reinterpret_cast<const DocumentNode *>(node);
    }

    ContentNode *parent_ = nullptr;
};

typedef std::shared_ptr<ContentNode> ContentNodePtr;
class ContainerNode: public ContentNode {
public:
    void addChild(ContentNodePtr child) {
        children_.push_back(child);
        child->parent_ = this;
    }

    virtual std::string tagName() const { return {}; }
    virtual bool shouldClose() const { return true; }
    std::vector<ContentNodePtr> children_;
};

typedef std::shared_ptr<ContainerNode> ContainerNodePtr;
class ForLoopBlockNode: public ContainerNode {
public:
    ForLoopBlockNode(identifier_list_t &&ids, ExpressionNodePtr target, ExpressionNodePtr cond = nullptr):
        ids_(ids), target_(target), condition_(cond) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override;
    std::string tagName() const override { return "for"; }
    void startElseBlock() {
        else_child_start_ = children_.size();
    }

    int else_child_start_ = -1;

    identifier_list_t ids_;
    ExpressionNodePtr target_, condition_;
};

class NamedBlockNode: public ContainerNode {
public:
    NamedBlockNode(const std::string &name): name_(name) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override;

    std::string tagName() const override { return "block"; }

    std::string name_;
};

typedef std::shared_ptr<NamedBlockNode> NamedBlockNodePtr;
class ExtensionBlockNode: public ContainerNode {
public:
    ExtensionBlockNode(ExpressionNodePtr src): parent_resource_(src) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override;

    std::string tagName() const override { return "extends"; }
    bool shouldClose() const { return false; }

    ExpressionNodePtr parent_resource_;
};

class IncludeBlockNode: public ContentNode {
public:
    IncludeBlockNode(ExpressionNodePtr source, bool ignore_missing, ExpressionNodePtr with_expr, bool only):
        source_(source), ignore_missing_(ignore_missing), with_(with_expr), only_flag_(only) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override;

    ExpressionNodePtr source_, with_;
    bool ignore_missing_, only_flag_;
};

class EmbedBlockNode: public ContainerNode {
public:
    EmbedBlockNode(ExpressionNodePtr source, bool ignore_missing, ExpressionNodePtr with_expr, bool only):
        source_(source), ignore_missing_(ignore_missing), with_(with_expr), only_flag_(only) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override;
    std::string tagName() const override { return "embed"; }

    ExpressionNodePtr source_, with_;
    bool ignore_missing_, only_flag_;
};

class WithBlockNode: public ContainerNode {
public:
    WithBlockNode(ExpressionNodePtr with_expr, bool only):
        with_(with_expr), only_flag_(only) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override;

    std::string tagName() const override { return "with"; }

    ExpressionNodePtr with_;
    bool only_flag_;
};

class AutoEscapeBlockNode: public ContainerNode {
public:
    AutoEscapeBlockNode(const std::string &mode): mode_(mode) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override;

    std::string tagName() const override { return "autoescape"; }
    std::string mode_;
};

class IfBlockNode: public ContainerNode {
public:
    IfBlockNode(ExpressionNodePtr target) { addBlock(target); }

    void eval(TemplateEvalContext &ctx, std::string &res) const override;

    std::string tagName() const override { return "if"; }

    void addBlock(ExpressionNodePtr ptr) {
        if ( !blocks_.empty() ) {
            blocks_.back().cstop_ = children_.size();
        }
        int cstart = blocks_.empty() ? 0 : blocks_.back().cstop_;
        blocks_.push_back({cstart, -1, ptr});
    }

    struct Block {
        int cstart_, cstop_;
        ExpressionNodePtr condition_;
    };

    std::vector<Block> blocks_;
};

class AssignmentBlockNode: public ContainerNode {
public:
    AssignmentBlockNode(const std::string &id, ExpressionNodePtr val): id_(id), val_(val) { }

    void eval(TemplateEvalContext &ctx, std::string &res) const override;

    std::string tagName() const override { return "set"; }
    bool shouldClose() const override { return false; }

    ExpressionNodePtr val_;
    std::string id_;
};

class FilterBlockNode: public ContainerNode {
public:
    FilterBlockNode(const std::string &name, key_val_list_t &&args = {}): name_(name), args_(args) { }

    void eval(TemplateEvalContext &ctx, std::string &res) const override;

    std::string tagName() const override { return "filter"; }

    std::string name_;
    key_val_list_t args_;
};

class MacroBlockNode: public ContainerNode {
public:
    MacroBlockNode(const std::string &name, identifier_list_t &&args): name_(name), args_(args) { }
    MacroBlockNode(const std::string &name): name_(name) { }

    void eval(TemplateEvalContext &ctx, std::string &res) const override;

    Variant call(TemplateEvalContext &ctx, const Variant &args);

    void mapArguments(const Variant &args, Variant::Object &ctx);

    std::string tagName() const override { return "macro"; }

    std::string name_;
    identifier_list_t args_;
};

class ImportBlockNode: public ContainerNode {
public:
    ImportBlockNode(ExpressionNodePtr source, const std::string &ns): source_(source), ns_(ns) { }
    ImportBlockNode(ExpressionNodePtr source, const key_alias_list_t &&mapping): source_(source),
        mapping_(mapping) { }

    void eval(TemplateEvalContext &ctx, std::string &res) const override;

    std::string tagName() const override { return "import"; }
    bool shouldClose() const { return false; }

    bool mapMacro(MacroBlockNode &n, std::string &name) const;

    std::string ns_;
    ExpressionNodePtr source_;
    key_alias_list_t mapping_;
};

class RawTextNode: public ContentNode {
public:
    RawTextNode(const std::string &text): text_(text) {}

    void eval(TemplateEvalContext &, std::string &res) const override {
        res.append(text_);
    }

    std::string text_;
};

class SubTextNode: public ContentNode {
public:
    SubTextNode(ExpressionNodePtr expr): expr_(expr), ContentNode() {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override;

    ExpressionNodePtr expr_;
};

class DocumentNode: public ContainerNode {
public:
    DocumentNode(TemplateRenderer &rdr): renderer_(rdr) {}

    void eval(TemplateEvalContext &ctx, std::string &res) const override {
        for( auto &&e: children_ )
            e->eval(ctx, res);
    }

    std::map<std::string, detail::ContentNodePtr> macro_blocks_;
    TemplateRenderer &renderer_;
};

typedef std::shared_ptr<DocumentNode> DocumentNodePtr;
} // namespace detail
} // namespace twig
} // namespace wspp
#endif