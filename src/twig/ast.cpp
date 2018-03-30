#include "ast.hpp"

#include <wspp/twig/functions.hpp>
#include <wspp/twig/renderer.hpp>
#include <wspp/twig/exceptions.hpp>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

using namespace std ;
using namespace wspp::util ;

namespace wspp { namespace twig {

namespace detail {


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
    if ( FunctionFactory::instance().hasFunction(name_) ) {
        return Variant(Variant::Function([=](const Variant &args, TemplateEvalContext &cc) {
            return FunctionFactory::instance().invoke(name_, args, cc) ;
        })) ;
    }
    else {
        auto it = ctx.data().find(name_) ;
        if ( it == ctx.data().end() ) return Variant::undefined() ;
        else return it->second ;
    }
}

Variant BinaryOperator::eval(TemplateEvalContext &ctx)
{
    Variant op1 = lhs_->eval(ctx) ;
    Variant op2 = rhs_->eval(ctx) ;

    if ( op1.isUndefined() || op1.isNull() )
        throw TemplateRuntimeException(str(boost::format("Undefined or null value on the left of %c operator") % (char)op_)) ;

    if ( op2.isUndefined() || op2.isNull() )
        throw TemplateRuntimeException(str(boost::format("Undefined or null value on the right of %c operator") % (char)op_)) ;

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

Variant InvokeTestNode::eval(TemplateEvalContext &ctx)
{
    Variant target = target_->eval(ctx) ;

    Variant v = filter_->eval(target, ctx) ;

    if ( v.type() != Variant::Type::Boolean )
        throw TemplateRuntimeException("test function returning non-boolean value") ;

    bool res = v.toBoolean() ;

    return ( positive_ ) ? res : !res ;
}



Variant FilterNode::eval(const Variant &target, TemplateEvalContext &ctx)
{
    Variant args ;
    args_->eval(args, ctx, boost::optional<Variant>(target)) ;
    return FunctionFactory::instance().invoke(name_, args, ctx) ;
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



void ForLoopBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    uint counter = 0 ;
    Variant target = target_->eval(ctx) ;
    int asize = target.length() ;

    string tmp ;

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
                    c->eval(cctx, tmp) ;
                } else if ( ids_.size() == 2 ) {
                    TemplateEvalContext cctx(tctx) ;
                    cctx.data()[ids_[0]] =  it.key() ;
                    cctx.data()[ids_[1]] =  it.value() ;
                    c->eval(cctx, tmp) ;
                }
            }
        }
    } else if ( else_child_start_ >= 0 ) {

        for( uint count = else_child_start_ ; count < children_.size() ; count ++ ) {
            children_[count]->eval(ctx, tmp) ;
        }
    }

    trim(tmp, res) ;
}

void IfBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    string tmp ;

    for( const Block &b: blocks_ ) {
        int c_start = b.cstart_ ;
        int c_stop = ( b.cstop_ == -1 ) ? children_.size() : b.cstop_ ;
        if (  !b.condition_ || b.condition_->eval(ctx).toBoolean() ) {
            for( int c = c_start ; c < c_stop ; c++ ) {
                children_[c]->eval(ctx, tmp) ;
            }
            return ;
        }
    }

    trim(tmp, res) ;
}

Variant TernaryExpressionNode::eval(TemplateEvalContext &ctx)
{
    if ( condition_->eval(ctx).toBoolean() )
        return positive_->eval(ctx) ;
    else return negative_ ? negative_->eval(ctx) : Variant::null() ;
}

void AssignmentBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    string tmp ;

    Variant val = val_->eval(ctx) ;
    TemplateEvalContext ectx(ctx) ;
    ectx.data()[id_] = val ;
    for( auto &&c: children_ )
        c->eval(ectx, tmp) ;

    trim(tmp, res) ;
}

void FilterBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    string block_res ;
    for( auto &&c: children_ )
        c->eval(ctx, block_res) ;
    string result = filter_->eval(block_res, ctx).toString() ;

    trim(std::move(result), res) ;
}



void ContentNode::trim(const string &src, string &out) const
{
    string::const_iterator start = src.begin(), end = src.end() ;

    if ( ws_ & WhiteSpace::TrimLeft )
        start = std::find_if_not(src.begin(), src.end(), [](unsigned char c){ return std::isspace(c); }) ;

    if ( ws_ & WhiteSpace::TrimRight )
        end = std::find_if_not(src.rbegin(), src.rend(), [](unsigned char c){ return std::isspace(c); }).base() ;

    out.append(start, end) ;
}

Variant InvokeFunctionNode::eval(TemplateEvalContext &ctx)
{
    Variant f = callable_->eval(ctx) ;

    Variant args ;
    args_->eval(args, ctx, boost::optional<Variant>()) ;

    if ( f.type() == Variant::Type::Function ) {
        return f.invoke(args, ctx) ;
    } else throw TemplateRuntimeException("function invocation of non-callable variable") ;
}


void NamedBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    string tmp ;
    auto it = ctx.blocks_.find(name_) ;
    if ( it != ctx.blocks_.end() ) {
        TemplateEvalContext cctx(ctx) ;
        cctx.data()["parent"] = Variant([&](const Variant &, TemplateEvalContext &cc) -> Variant {
            string pp ;
            for( auto &&c: children_ ) {
                c->eval(cc, pp) ;
            }
            return Variant(pp) ;
        }) ;

        for( auto &&c: it->second->children_ ) {
            c->eval(cctx, tmp) ;
        }
    }
    else {
        for( auto &&c: children_ ) {
            c->eval(ctx, tmp) ;
        }
    }

    trim(tmp, res) ;
}

void ExtensionBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    string resource = parent_resource_->eval(ctx).toString() ;

    TemplateRenderer &rdr = ctx.rdr_ ;

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



void MacroBlockNode::eval(TemplateEvalContext &, string &) const
{

}

// map passed arguments to context variables with the same name as macro parameters
void MacroBlockNode::mapArguments(const Variant &args, Variant::Object &ctx, Variant::Array &arg_list)
{
    auto it = args.begin() ;
    for( auto &&arg_name: args_ ) {
        Variant val = Variant::null() ;
        if ( it != args.end()  )
            val = *it++ ;

        ctx[arg_name] = val ; // replace current contex variables with the same name
        arg_list.push_back(val) ;
    }

    // add the rest of non-mapped arguments to arglist array

    while ( it != args.end() ) {
        arg_list.push_back(*it++) ;
    }
}

void ImportBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    DocumentNodePtr doc ;

    // if _self was provided then source should be null

    if ( source_ ) {
        string resource = source_->eval(ctx).toString() ;

        TemplateRenderer &rdr = ctx.rdr_ ;

        doc = rdr.compile(resource) ;
    }

    TemplateEvalContext pctx(ctx) ;

    // create closures corresponding to each macro and add them to the namespace variable
    // of the current context

    Variant::Object closures ;

    for( auto &&m: (doc) ? doc->macro_blocks_ : root()->macro_blocks_ ) {

        std::shared_ptr<MacroBlockNode> p_macro = std::dynamic_pointer_cast<MacroBlockNode>(m.second) ;
        if ( p_macro ) {

            string mapped_name ;
            if ( !mapMacro(*p_macro, mapped_name) ) continue ; // if not imported

            auto macro_fn = [&, p_macro](const Variant &args, TemplateEvalContext &tctx) -> Variant {
                // in this implementation context is always provided so no need for providing the _context parameter

                TemplateEvalContext mctx(tctx) ;
                Variant::Array arg_list ;
                p_macro->mapArguments(args.at("args"), mctx.data(), arg_list) ;
                mctx.data()["varargs"] = arg_list ;

                string tmp, out ;

                for( auto &&c: p_macro->children_ ) {
                    c->eval(mctx, tmp) ;
                }

                p_macro->trim(tmp, out) ;
                return out ;
            };

            closures.insert({mapped_name, Variant::Function(macro_fn)}) ;
        }
    }

    if ( !ns_.empty() ) pctx.data()[ns_] = Variant(closures) ;
    else {
        for( auto &e: closures ) {
            pctx.data()[e.first] = e.second ;
        }
    }

    for( auto &&c: children_ ) {
        c->eval(pctx, res) ;
    }

}

bool ImportBlockNode::mapMacro(MacroBlockNode &n, string &name) const {

    // only namespace given import all macros using their name
    if ( mapping_.empty() ) {
        name = n.name_ ;
        return true ;
    }

    // check mapping list if the macro has been imported
    auto it = std::find_if(mapping_.begin(), mapping_.end(), [&n](const ImportKeyAlias &e) { return e.key_ == n.name_ ; } ) ;
    if ( it == mapping_.end() ) return false ; // not found ignore
    name = (*it).alias_.empty() ? n.name_ : (*it).alias_ ;
    return true ;
}

void IncludeBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    // get the list of templates that we are going to check

    vector<string> templates ;

    Variant source = source_->eval(ctx) ;

    if ( source.isArray() ) {
        for( auto &&e: source )
            templates.emplace_back(e.toString()) ;
    }
    else
        templates.emplace_back(source.toString()) ;

    // try to load one of the templates

    DocumentNodePtr doc ;

    for( auto &&tmpl: templates ) {
        try {
            doc = ctx.rdr_.compile(tmpl) ;
            break ;
        }
        catch ( ... ) {

        }
    }

    // check whether not found

    if ( !doc ) {
        if ( !ignore_missing_ ) // non found
            throw TemplateRuntimeException("Failed to load included template") ;
        else
            return ;
    }

    // template is compiled so render it with the appropriate context

    Variant::Object ctx_extension ;

    // if there is a with statment we collect the variables

    if ( with_ ) {
        Variant with = with_->eval(ctx) ;
        if ( with.isObject() ) {
            for ( auto it = with.begin() ; it != with.end() ; ++it ) {
                ctx_extension[it.key()] = it.value() ;
            }
        }
    }

    // create new context either inheriting parent one or empty and extend with key/values if any

    string tmp ;

    if ( only_flag_ ) {
        TemplateEvalContext cctx(ctx.rdr_, {}) ;
        cctx.data().insert(ctx_extension.begin(), ctx_extension.end()) ;
        doc->eval(cctx, tmp) ;
    } else {
        TemplateEvalContext cctx(ctx) ;
        for( auto &&e: ctx_extension )
            cctx.data()[e.first] = e.second ;
        doc->eval(cctx, tmp) ;
    }

    trim(tmp, res) ;
}


void WithBlockNode::eval(TemplateEvalContext &ctx, string &res) const
{
    Variant::Object ctx_extension ;

    // if there is a with statment we collect the variables

    if ( with_ ) {
        Variant with = with_->eval(ctx) ;
        if ( with.isObject() ) {
            for ( auto it = with.begin() ; it != with.end() ; ++it ) {
                ctx_extension[it.key()] = it.value() ;
            }
        }
    }

    // create new context either inheriting parent one or empty and extend with key/values if any

    string tmp ;

    if ( only_flag_ ) {
        TemplateEvalContext cctx(ctx.rdr_, {}) ;
        cctx.data().insert(ctx_extension.begin(), ctx_extension.end()) ;
        for( auto &&c: children_ )
            c->eval(cctx, tmp) ;
    } else {
        TemplateEvalContext cctx(ctx) ;
        for( auto &&e: ctx_extension )
            cctx.data()[e.first] = e.second ;
        for( auto &&c: children_ )
            c->eval(cctx, tmp) ;
    }

    trim(tmp, res) ;

}


}
 // namespace detail

void TemplateEvalContext::addBlock(detail::NamedBlockNodePtr node) {
    blocks_.insert({node->name_, node}) ;
}

} // namespace twig
} // namespace wspp
