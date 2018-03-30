#include <wspp/twig/functions.hpp>
#include <wspp/twig/exceptions.hpp>
#include <wspp/twig/renderer.hpp>

#include <boost/algorithm/string.hpp>

using namespace wspp::util ;
using namespace std ;

namespace wspp { namespace twig {


Variant FunctionFactory::invoke(const string &name, const Variant &args, TemplateEvalContext &ctx)
{
    auto it = functions_.find(name) ;
    if ( it == functions_.end() )
        throw TemplateRuntimeException("Unknown function or filter: " + name) ;
    return it->second(args, ctx) ;
}

void FunctionFactory::registerFunction(const string &name, const TemplateFunction &f)
{
    functions_.emplace(name, f);

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

static Variant _join(const Variant &args, TemplateEvalContext &) {
    Variant::Array unpacked ;
    if ( unpack_args(args, { { "string_list", true }, { "sep", false }, {"key", false } },  unpacked) ) {

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
}

static Variant _lower(const Variant &args, TemplateEvalContext &) {
    Variant::Array unpacked ;
    if ( unpack_args(args, { { "str", true }}, unpacked) )
        return boost::to_lower_copy(unpacked[0].toString()) ;
}

static Variant _upper(const Variant &args, TemplateEvalContext &) {
    Variant::Array unpacked ;
    if ( unpack_args(args, { { "str", true }}, unpacked) )
        return boost::to_upper_copy(unpacked[0].toString()) ;
}

static Variant _default(const Variant &args, TemplateEvalContext &) {
    Variant::Array unpacked ;
    if ( unpack_args(args, { { "str", true }, {"default", true} }, unpacked) )
        return unpacked[0].isNull() ? unpacked[1] : unpacked[0] ;
}

static Variant _escape(const Variant &args, TemplateEvalContext &) {
    Variant::Array unpacked ;
    if ( unpack_args(args, { { "str", true }}, unpacked) )
        return unpacked[0].toString() ;
}

static Variant _include(const Variant &args, TemplateEvalContext &ctx) {
    Variant::Array unpacked ;
    if ( unpack_args(args, { { "file", true }}, unpacked) ) {
        string resource = unpacked[0].toString() ;
        return ctx.rdr_.render(resource, ctx.data()) ;
    }

}

static Variant _defined(const Variant &args, TemplateEvalContext &ctx) {
    Variant::Array unpacked ;
    if ( unpack_args(args, { { "var", true }}, unpacked) ) {
        return !(unpacked[0].isUndefined() ) ;
    }
}

FunctionFactory::FunctionFactory() {
    registerFunction("join", _join);
    registerFunction("lower", _lower);
    registerFunction("upper", _upper);
    registerFunction("default", _default);
    registerFunction("e", _escape);
    registerFunction("escape", _escape);
    registerFunction("include", _include);
    registerFunction("defined", _defined);
}

bool FunctionFactory::hasFunction(const string &name)
{
    return functions_.count(name) ;
}

} // namespace twig
} // namespace wspp
