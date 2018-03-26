#include "template_ast.hpp"

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
    else if ( op_ == '.' )
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

Variant ApplyFilterNode::eval(TemplateEvalContext &ctx)
{
    Variant target = target_->eval(ctx) ;

    return filter_->eval(target, ctx) ;
}



Variant FilterNode::eval(const Variant &target, TemplateEvalContext &ctx)
{
    Variant::Array args ;
    args.push_back(target) ;
    evalArgs(args, ctx) ;
    return dispatch(name_, args) ;
}

void FilterNode::evalArgs(Variant::Array &args, TemplateEvalContext &ctx) const
{
    if ( !args_ ) return ;

    for ( auto &&e: args_->children() )
        args.push_back(e->eval(ctx)) ;
}

Variant FilterNode::dispatch(const string &fname, const Variant::Array &args)
{
    if ( fname == "join" ) {
        string sep = ( args.size() > 1 ) ? args[1].toString() : "" ;
        string key = ( args.size() == 3 ) ? args[2].toString() : "" ;

        bool is_first = true ;
        string res ;
        for( auto &i: args[0] ) {
            if ( !is_first ) res.append(sep) ;
            if ( !key.empty() )
                res.append(i.at(key).toString()) ;
            else
                res.append(i.toString()) ;
            is_first = false ;
        }
        return res ;
    } else if ( fname == "lower" ) {
        return boost::to_lower_copy(args[0].toString()) ;
    } else if ( fname == "upper" ) {
        return boost::to_upper_copy(args[0].toString()) ;
    }
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


}

