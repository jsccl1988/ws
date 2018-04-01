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


static bool compare_numbers(int64_t lhs, int64_t rhs, ComparisonPredicate::Type op) {
    switch ( op ) {
    case ComparisonPredicate::Equal:
        return lhs == rhs ;
    case ComparisonPredicate::NotEqual:
        return lhs != rhs ;
    case ComparisonPredicate::Less:
        return lhs < rhs ;
    case ComparisonPredicate::LessOrEqual:
        return lhs <= rhs ;
    case ComparisonPredicate::GreaterOrEqual:
        return lhs >= rhs ;
    case ComparisonPredicate::Greater:
        return lhs > rhs ;
    }
}

static bool compare_numbers(const Variant &lhs, const Variant &rhs, ComparisonPredicate::Type op) {

    if ( ( lhs.type() == Variant::Type::Integer || lhs.type() == Variant::Type::Boolean) && rhs.type() == Variant::Type::Float ) {
        return compare_numbers(lhs.toFloat(), rhs.toFloat(), op) ;
    } else if ( lhs.type() == Variant::Type::Float && ( rhs.type() == Variant::Type::Integer || rhs.type() == Variant::Type::Boolean ) ) {
        return compare_numbers(lhs.toFloat(), rhs.toFloat(), op) ;
    } else
        return compare_numbers(lhs.toInteger(), rhs.toInteger(), op) ;
}

static bool variant_compare(const Variant &lhs, const Variant &rhs, ComparisonPredicate::Type op) {

    if ( lhs.isString() && rhs.isString() ) {
        string ls = lhs.toString(), rs = rhs.toString() ;
        int res = ls.compare(rs) ;
        switch ( op ) {
        case ComparisonPredicate::Equal:
            return res == 0 ;
        case ComparisonPredicate::NotEqual:
            return res != 0 ;
        case ComparisonPredicate::Less:
            return res < 0 ;
        case ComparisonPredicate::LessOrEqual:
            return res <= 0 ;
        case ComparisonPredicate::GreaterOrEqual:
            return res >= 0 ;
        case ComparisonPredicate::Greater:
            return res > 0 ;
        }
    }

    if ( lhs.isNumber() && rhs.isNumber() ) {
        return compare_numbers(lhs, rhs, op) ;
    } else if ( lhs.isNumber() && rhs.isString() ) {
        return compare_numbers(lhs, rhs.toNumber(), op ) ;
    } else if ( lhs.isString() && rhs.isNumber() ) {
        return compare_numbers(lhs.toNumber(), rhs, op ) ;
    } else return compare_numbers((int64_t)&lhs, (int64_t)&rhs, op) ;

}

Variant ComparisonPredicate::eval(TemplateEvalContext &ctx)
{
    Variant lhs = lhs_->eval(ctx) ;
    Variant rhs = rhs_->eval(ctx) ;

    if ( lhs.isNull() || rhs.isNull() ) return false ;

    return variant_compare(lhs, rhs, op_);
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

static int64_t arithemtic(int64_t lhs, int64_t rhs, char op) {
    switch ( op ) {
    case '+':
        return lhs + rhs ;
    case '-':
        return lhs - rhs ;
    case '*':
        return lhs * rhs ;
    case '/':
        return ( rhs ) ? (lhs / rhs) : 0 ;
    case '%':
        return ( rhs ) ? (lhs % rhs) : 0 ;
    }
}

static double arithemtic(double lhs, double rhs, char op) {
    switch ( op ) {
    case '+':
        return lhs + rhs ;
    case '-':
        return lhs - rhs ;
    case '*':
        return lhs * rhs ;
    case '/':
        return ( rhs != 0 ) ? (lhs / rhs) : 0.0 ;
    case '%':
        return ( (int64_t)rhs != 0) ? ((int64_t)lhs % (int64_t)rhs) : 0 ;
    }
}


static Variant arithmetic(const Variant &lhs, const Variant &rhs, char op) {
    if ( ( lhs.type() == Variant::Type::Integer || lhs.type() == Variant::Type::Boolean) && rhs.type() == Variant::Type::Float ) {
        return arithmetic(lhs.toFloat(), rhs.toFloat(), op) ;
    } else if ( lhs.type() == Variant::Type::Float && ( rhs.type() == Variant::Type::Integer || rhs.type() == Variant::Type::Boolean ) ) {
        return arithmetic(lhs.toFloat(), rhs.toFloat(), op) ;
    } else
        return arithmetic(lhs.toInteger(), rhs.toInteger(), op) ;

}

Variant BinaryOperator::eval(TemplateEvalContext &ctx)
{
    Variant op1 = lhs_->eval(ctx) ;
    Variant op2 = rhs_->eval(ctx) ;

    if ( op1.isUndefined() || op1.isNull() )
        throw TemplateRuntimeException(str(boost::format("Undefined or null value on the left of %c operator") % (char)op_)) ;

    if ( op2.isUndefined() || op2.isNull() )
        throw TemplateRuntimeException(str(boost::format("Undefined or null value on the right of %c operator") % (char)op_)) ;

    switch ( op_ ) {
    case '+':
    case '-':
    case '*':
    case '/':
        return arithmetic(op1.toNumber(), op2.toNumber(), op_) ;
    case '~':
        return op1.toString() + op2.toString() ;
    }
/*
    if ( op_ == '+' )
    {
        return arithmetic(op1.toNumber(), op2.toNumber() ;
    }
    else if ( op_ == '-' )
    {
        return op1.toNumber() - op2.toNumber() ;
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
    */
}


Variant UnaryOperator::eval(TemplateEvalContext &ctx)
{
    Variant val = rhs_->eval(ctx) ;

    if ( op_ == '-' ) {
        return arithmetic(0, val, '-') ;
    }
    else return val ;

}


Variant ArrayNode::eval(TemplateEvalContext &ctx)
{
    Variant::Array a ;

    for ( ExpressionNodePtr e: elements_ ) {
        a.push_back(e->eval(ctx)) ;
    }

    return a ;
}

Variant DictionaryNode::eval(TemplateEvalContext &ctx)
{
    Variant::Object o ;

    for ( key_val_t &kv: elements_ ) {
        o.insert({kv.first, kv.second->eval(ctx)}) ;
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
        return a.at(index.toInteger()) ;
}

Variant AttributeIndexingNode::eval(TemplateEvalContext &ctx)
{
    Variant o = dict_->eval(ctx) ;
    return o.at(key_) ;
}


static void evalArgs(const key_val_list_t &input_args, Variant &args, TemplateEvalContext &ctx, const Variant &extra = Variant::undefined())
{
    Variant::Array pos_args ;

    if ( !extra.isUndefined() )
        pos_args.push_back(extra) ;

    Variant::Object kv_args ;

    for ( auto &&e: input_args ) {
        if ( e.first.empty() )
            pos_args.push_back(e.second->eval(ctx)) ;
        else
            kv_args.insert({e.first, e.second->eval(ctx)}) ;

    }

    args = Variant::Object{{"args", pos_args}, {"kw", kv_args}} ;
}



static Variant evalFilter(const string &name, const key_val_list_t &args, const Variant &target, TemplateEvalContext &ctx) {

    Variant evargs ;
    evalArgs(args, evargs, ctx, target) ;
    return FunctionFactory::instance().invoke(name, evargs, ctx) ;
}

Variant InvokeFilterNode::eval(TemplateEvalContext &ctx)
{
    Variant target = target_->eval(ctx) ;
    return evalFilter(name_, args_, target, ctx) ;
}

Variant InvokeTestNode::eval(TemplateEvalContext &ctx) {
    Variant target = target_->eval(ctx) ;

    Variant v = evalFilter(name_, args_, target, ctx) ;

    if ( v.type() != Variant::Type::Boolean )
        throw TemplateRuntimeException("test function returning non-boolean value") ;

    bool res = v.toBoolean() ;

    return ( positive_ ) ? res : !res ;
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

            if ( condition_ && !condition_->eval(ctx).toBoolean() ) continue ;

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
            break ;
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

    string result = evalFilter(name_, args_, block_res, ctx).toString() ;

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
    evalArgs(args_, args, ctx) ;

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
    auto it = std::find_if(mapping_.begin(), mapping_.end(), [&n](const key_alias_t &e) { return e.first == n.name_ ; } ) ;
    if ( it == mapping_.end() ) return false ; // not found ignore
    name = (*it).second.empty() ? n.name_ : (*it).second ;
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
            throw TemplateRuntimeException("Failed to load included template: " + templates[0]) ;
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

static bool compare_numbers(double lhs, double rhs, ComparisonPredicate::Type op) {
    switch ( op ) {
    case ComparisonPredicate::Equal:
        return lhs == rhs ;
    case ComparisonPredicate::NotEqual:
        return lhs != rhs ;
    case ComparisonPredicate::Less:
        return lhs < rhs ;
    case ComparisonPredicate::LessOrEqual:
        return lhs <= rhs ;
    case ComparisonPredicate::GreaterOrEqual:
        return lhs >= rhs ;
    case ComparisonPredicate::Greater:
        return lhs > rhs ;
    }
}


Variant ContainmentNode::eval(TemplateEvalContext &ctx)
{
    Variant lhs = lhs_->eval(ctx) ;
    Variant rhs = lhs_->eval(ctx) ;

    if ( !lhs.isPrimitive() || !rhs.isArray() )
        throw TemplateRuntimeException("wrong type of values on containment operaror") ;

    for( auto &&e: rhs ) {
        if ( variant_compare(lhs, rhs, ComparisonPredicate::Equal) ) return true ;
    }

}

MatchesNode::MatchesNode(ExpressionNodePtr lhs, const string &rx, bool positive): lhs_(lhs), positive_(positive) {

    if ( rx.size() < 2 ) throw TemplateCompileException("empty regex string") ;
    const char *p = rx.c_str(), *q = p + rx.length() - 1 ;
    char mod = 0 ;

    if ( *q != *p ) { mod = *q ; --q ; }
    if ( *q != *p )
        throw TemplateCompileException("unmatched delimiters in regex") ;

    boost::regex::flag_type flags = boost::regex::perl ;/*| boost::regex::no_mod_m*/;

    switch ( mod ) {
    case 0:
        break ;
    case 's':
        flags |= boost::regex::mod_s ;
        break ;
    case 'i':
        flags |= boost::regex::icase ;
        break ;
    case 'm':
        flags |= boost::regex::no_mod_m ;
        break ;
    case 'x':
        flags |= boost::regex::mod_x ;
        break ;
    }

    ++p ;
    rx_.assign(p, q) ;


}

Variant MatchesNode::eval(TemplateEvalContext &ctx)
{
    string val = lhs_->eval(ctx).toString() ;

    bool res = boost::regex_match(val, rx_)  ;

    return (bool)res ;
}


}
 // namespace detail

void TemplateEvalContext::addBlock(detail::NamedBlockNodePtr node) {
    blocks_.insert({node->name_, node}) ;
}

} // namespace twig
} // namespace wspp
