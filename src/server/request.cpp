#include <wspp/server/request.hpp>
#include <wspp/server/route.hpp>

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

using namespace std ;

namespace wspp { namespace server {

bool Request::matches(const string &method, const string &pattern, Dictionary &attributes) const
{
    return matchesMethod(method) && Route(pattern).matches(path_, attributes) ;
}

bool Request::matches(const string &method, const string &pattern) const
{
    Dictionary attributes ;
    return matchesMethod(method) && Route(pattern).matches(path_, attributes) ;
}

bool Request::matches(const string &method, const Route &pattern, Dictionary &attributes) const
{
    return matchesMethod(method) && pattern.matches(path_, attributes) ;
}

bool Request::matches(const string &method, const Route &pattern) const
{
    Dictionary attributes ;
    return matchesMethod(method) && pattern.matches(path_, attributes) ;
}

bool Request::matchesMethod(const string &method) const
{
    vector<string> methods ;
    boost::split( methods, method, boost::is_any_of(" |"), boost::token_compress_on );
    return std::find(methods.begin(), methods.end(), method_) != methods.end() ;
}

} // namespace server
} // namespace wspp
