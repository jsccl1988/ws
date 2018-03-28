#include "template_ast.hpp"
#include "template_renderer.hpp"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

using namespace std ;
using namespace wspp::util ;

namespace ast {


Variant BooleanOperator::eval(TemplateEvalContext &ctx)
{
    switch ( op_ ) {
    case And:
        return ( lhs_->eval(ctx).toBoolean() && rhs_->eval(ctx).toBoolean() ) ;
    case Or:
        return ( lhs_->eval(ctx).toBoolean() || rhs_->eval(ctx).toBoolean() ) ;
    case Not: //?
        return !( lhs_->eval(ctx).toBoolean() ) ;
    }
}

Variant ComparisonPredicate::eval(TemplateEvalContext &ctx)
{
    Variant lhs = lhs_->eval(ctx) ;
    Variant rhs = rhs_->eval(ctx) ;

    if ( lhs.isNull() || rhs.isNull() ) return false ;

    switch ( op_ ) {
    case Equal:
    {
        if ( lhs.isString() && rhs.isString() )
            return Variant(lhs.toString() == rhs.toString()) ;
        else return Variant(lhs.toNumber() == rhs.toNumber()) ;
    }
    case NotEqual:
        if ( lhs.isString() && rhs.isString() )
            return Variant(lhs.toString() != rhs.toString()) ;
        else return Variant(lhs.toNumber() != rhs.toNumber()) ;
    case Less:
        return lhs.toNumber() < rhs.toNumber() ;
    case Greater:
        return lhs.toNumber() > rhs.toNumber() ;
    case LessOrEqual:
        return lhs.toNumber() <= rhs.toNumber() ;
    case GreaterOrEqual:
        return lhs.toNumber() >= rhs.toNumber() ;
    }

    return Variant() ;
}



Variant IdentifierNode::eval(TemplateEvalContext &ctx)
{
    auto it = ctx.data().find(name_) ;
    if ( it == ctx.data().end() ) return Variant::null() ;
    else return it->second ;
}



Variant BinaryOperator::eval(TemplateEvalContext &ctx)
{
    Variant op1 = lhs_->eval(ctx) ;
    Variant op2 = rhs_->eval(ctx) ;

    if ( op_ == '+' )
    {
        return op1.toNumber() + op2.toNumber() ;
    }
    else if ( op_ == '-' )
    {
        return op1.toNumber() + op2.toNumber() ;
    }
    else if ( op_ == '~' )
    {
        return op1.toString() + op2.toString() ;
    }
    else if ( op_ == '*' )
    {
        return op1.toNumber() * op2.toNumber() ;
    }
    else if ( op_ == '/' ) {
        if ( op2.toNumber() == 0.0 ) return Variant::null() ;
        else return op1.toNumber()/op2.toNumber() ;
    }
}


Variant UnaryOperator::eval(TemplateEvalContext &ctx)
{
    Variant val = rhs_->eval(ctx) ;

    if ( op_ == '-' ) {
        return -val.toNumber() ;
    }
    else return val ;

}


Variant ArrayNode::eval(TemplateEvalContext &ctx)
{
    Variant::Array a ;

    for ( ExpressionNodePtr e: elements_->children() ) {
        a.push_back(e->eval(ctx)) ;
    }

    return a ;
}

Variant DictionaryNode::eval(TemplateEvalContext &ctx)
{
    Variant::Object o ;

    for ( KeyValNodePtr kv: elements_->children() ) {
        o.insert({kv->key_, kv->val_->eval(ctx)}) ;
    }

    return o ;
}

Variant SubscriptIndexingNode::eval(TemplateEvalContext &ctx)
{
    Variant index = index_->eval(ctx) ;
    Variant a = array_->eval(ctx) ;

    if ( index.isString() )
        return a.at(index.toString()) ;
    else
        return a.at(index.toNumber()) ;
}

Variant AttributeIndexingNode::eval(TemplateEvalContext &ctx)
{
    Variant o = dict_->eval(ctx) ;
    return o.at(key_) ;
}

Variant InvokeFilterNode::eval(TemplateEvalContext &ctx)
{
    Variant target = target_->eval(ctx) ;

    return filter_->eval(target, ctx) ;
}



Variant FilterNode::eval(const Variant &target, TemplateEvalContext &ctx)
{
    Variant args ;
    if ( args_ )
        args_->eval(args, ctx, boost::optional<Variant>(target)) ;
    else
        args = Variant::Object{{"args", Variant::Array{{target}}}, { "kw", Variant::Object{{}}}} ;

    return dispatch(name_, args) ;
}


void FunctionArguments::eval(Variant &args, TemplateEvalContext &ctx, const boost::optional<Variant> &extra) const
{
    Variant::Array pos_args ;

    if ( extra.is_initialized() )
        pos_args.push_back(extra.get()) ;

    Variant::Object kv_args ;

    for ( auto &&e: children() ) {
        if ( e->name_.empty() )
            pos_args.push_back(e->val_->eval(ctx)) ;
        else
            kv_args.insert({e->name_, e->val_->eval(ctx)}) ;

    }

    args = Variant::Object{{"args", pos_args}, {"kw", kv_args}} ;
}

// unpack passed arguments into an array checking if all required arguments have been passed
// the known arguments are supplied by named_args map which provides the argument name and whether it is required or not

static bool unpack_args(const Variant &args, const std::vector<pair<std::string, bool>> &named_args, Variant::Array &res) {

    uint n_args = named_args.size() ;

    res.resize(n_args, Variant::null()) ;

    std::vector<bool> provided(n_args, false) ;

    const Variant &pos_args = args["args"] ;

    for ( uint pos = 0 ; pos < n_args && pos < pos_args.length() ; pos ++ )  {
        res[pos] = pos_args.at(pos) ;
        provided[pos] = true ;
    }

    const Variant &kw_args = args["kw"] ;

    for ( auto it = kw_args.begin() ; it != kw_args.end() ; ++it ) {
        string key = it.key() ;
        const Variant &val = it.value() ;

        for( uint k=0 ; k<named_args.size() ; k++ ) {
            if ( key == named_args[k].first && !provided[k] ) {
                res[k] = val ;
                provided[k] = true ;
            }

        }
    }

    uint required = std::count_if(named_args.begin(), named_args.end(), [](const pair<string, bool> &b) { return b.second ;});

    return std::count(provided.begin(), provided.end(), true) >= required ;
}

Variant FilterNode::dispatch(const string &fname, const Variant &args)
{
    if ( fname == "join" ) {
        Variant::Array unpacked ;
        if ( unpack_args(args, { { "str", true }, { "sep", false }, {"key", false } },  unpacked) ) {

            string sep = ( unpacked[1].isNull() ) ? "" : unpacked[1].toString() ;
            string key = ( unpacked[2].isNull() ) ? "" : unpacked[2].toString() ;

            bool is_first = true ;
            string res ;
            for( auto &i: unpacked[0] ) {
                if ( !is_first ) res.append(sep) ;
                if ( !key.empty() )
                    res.append(i.at(key).toString()) ;
                else
                    res.append(i.toString()) ;
                is_first = false ;
            }
            return res ;
        }
    } else if ( fname == "lower" ) {
        Variant::Array unpacked ;
        if ( unpack_args(args, { { "str", true }}, unpacked) )
            return boost::to_lower_copy(unpacked[0].toString()) ;
    } else if ( fname == "upper" ) {
        Variant::Array unpacked ;
        if ( unpack_args(args, { { "str", true }}, unpacked) )
            return boost::to_upper_copy(unpacked[0].toString()) ;
    } else if ( fname == "default" ) {
        Variant::Array unpacked ;
        if ( unpack_args(args, { { "str", true }, {"default", true} }, unpacked) )
            return unpacked[0].isNull() ? unpacked[1] : unpacked[0] ;
    } else if ( fname == "e" ) {
        Variant::Array unpacked ;
        if ( unpack_args(args, { { "str", true }}, unpacked) )
            return unpacked[0].toString() ;
    }

    return Variant::null() ;
}

void ForLoopBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    uint counter = 0 ;
    Variant target = target_->eval(ctx) ;
    int asize = target.length() ;

    if ( asize > 0 ) {
        int child_count = ( else_child_start_ < 0 ) ? children_.size() : else_child_start_ ;
        for ( auto it = target.begin() ; it != target.end() ; ++it, counter++  ) {
            TemplateEvalContext tctx(ctx) ;

            Variant::Object loop{ {"index0", counter},
                                  {"index", counter+1},
                                  {"revindex0", asize - counter - 1},
                                  {"revindex1", asize - counter},
                                  {"first", counter == 0},
                                  {"last", counter == asize-1},
                                  {"length", asize}
                                } ;
            tctx.data()["loop"] = loop ;

            uint i = 0 ;
            for( auto &&c: children_ ) {
                if ( ++i > child_count ) break ;
                if ( ids_.size() == 1 ) {
                    TemplateEvalContext cctx(tctx) ;
                    cctx.data()[ids_[0]] = *it ;
                    c->eval(cctx, res) ;
                } else if ( ids_.size() == 2 ) {
                    TemplateEvalContext cctx(tctx) ;
                    cctx.data()[ids_[0]] =  it.key() ;
                    cctx.data()[ids_[1]] =  it.value() ;
                    c->eval(cctx, res) ;
                }
            }
        }
    } else if ( else_child_start_ >= 0 ) {

        for( uint count = else_child_start_ ; count < children_.size() ; count ++ ) {
            children_[count]->eval(ctx, res) ;
        }
    }
}

void IfBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    for( const Block &b: blocks_ ) {
        int c_start = b.cstart_ ;
        int c_stop = ( b.cstop_ == -1 ) ? children_.size() : b.cstop_ ;
        if (  !b.condition_ || b.condition_->eval(ctx).toBoolean() ) {
            for( int c = c_start ; c < c_stop ; c++ ) {
                children_[c]->eval(ctx, res) ;
            }
            return ;
        }
    }

}

Variant TernaryExpressionNode::eval(TemplateEvalContext &ctx)
{
    if ( condition_->eval(ctx).toBoolean() )
        return positive_->eval(ctx) ;
    else return negative_ ? negative_->eval(ctx) : Variant::null() ;
}

void AssignmentBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    Variant val = val_->eval(ctx) ;
    TemplateEvalContext ectx(ctx) ;
    ectx.data()[id_] = val ;
    for( auto &&c: children_ )
        c->eval(ectx, res) ;
}

void FilterBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    string block_res ;
    for( auto &&c: children_ )
        c->eval(ctx, block_res) ;
    string result = filter_->eval(block_res, ctx).toString() ;
    res.append(std::move(result)) ;
}

string ContentNode::trim(const string &src) const
{
    if ( ws_ == WhiteSpace::TrimLeft )
        return boost::trim_left_copy(src) ;
    else if ( ws_ == WhiteSpace::TrimRight )
        return boost::trim_right_copy(src) ;
    else if ( ws_ == WhiteSpace::TrimBoth )
        return boost::trim_copy(src) ;
    else
        return std::move(src) ;
}

Variant InvokeFunctionNode::eval(TemplateEvalContext &ctx)
{
    Variant f = callable_->eval(ctx) ;
    if ( f.type() != Variant::Type::Function )
        return nullptr ;
    else {
        Variant args ;
        if ( args_ )
            args_->eval(args, ctx, boost::optional<Variant>()) ;
        return f.invoke(args) ;
    }
}

void NamedBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    auto it = ctx.blocks_.find(name_) ;
    if ( it != ctx.blocks_.end() ) {
        TemplateEvalContext cctx(ctx) ;
        cctx.data()["parent"] = Variant([&](const Variant &) -> Variant {
            string pp ;
            for( auto &&c: children_ ) {
                c->eval(cctx, pp) ;
            }
            return Variant(pp) ;
        }) ;

        for( auto &&c: it->second->children_ ) {
            c->eval(cctx, res) ;
        }
    }
    else {
        for( auto &&c: children_ ) {
            c->eval(ctx, res) ;
        }
    }
}

void ExtensionBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    string resource = parent_resource_->eval(ctx).toString() ;

    TemplateRenderer &rdr = *ctx.rdr_ ;

    auto parent = rdr.compile(resource) ;

    TemplateEvalContext pctx(ctx) ;

    // fill context with block definitions

    for( auto &&c: children_ ) {
        NamedBlockNodePtr block = std::dynamic_pointer_cast<NamedBlockNode>(c) ;
        if ( block )
            pctx.addBlock(block) ;
    }

    parent->eval(pctx, res) ;
}

void TemplateEvalContext::addBlock(NamedBlockNodePtr node) {
    blocks_.insert({node->name_, node}) ;
}

void MacroBlockNode::eval(TemplateEvalContext &, string &) const
{

}

void MacroBlockNode::mapArguments(const Variant &args, Variant::Object &ctx, Variant::Array &arg_list)
{
    auto it = args.begin() ;
    for( auto &&arg_name: args_ ) {
        Variant val = Variant::null() ;
        if ( it != args.end()  )
            val = *it++ ;

        ctx.insert({arg_name, val}) ;
        arg_list.push_back(val) ;
    }

    while ( it != args.end() ) {
        arg_list.push_back(*it++) ;
    }
}

void ImportBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    DocumentNodePtr doc = ctx.doc_ ;

    if ( source_ ) {
        string resource = source_->eval(ctx).toString() ;

        TemplateRenderer &rdr = *ctx.rdr_ ;

        doc = rdr.compile(resource) ;

    }

    TemplateEvalContext pctx(ctx) ;

    Variant::Object closures ;

    for( auto &&m: doc->macro_blocks_ ) {

        std::shared_ptr<MacroBlockNode> p_macro = std::dynamic_pointer_cast<MacroBlockNode>(m.second) ;
        if ( p_macro ) {
            auto macro_fn = [&, p_macro](const Variant &args) -> Variant {
                TemplateEvalContext tctx(ctx) ;
                Variant::Array arg_list ;
                p_macro->mapArguments(args.at("args"), tctx.data(), arg_list) ;
                tctx.data()["varargs"] = arg_list ;

                string res ;
                for( auto &&c: p_macro->children_ ) {
                    c->eval(tctx, res) ;
                }

                return res ;
            };

            closures.insert({p_macro->name_, Variant::function_t(macro_fn)}) ;
        }
    }

    pctx.data().insert({ns_, Variant(closures)}) ;

    for( auto &&c: children_ ) {
        c->eval(pctx, res) ;
    }

}



}

