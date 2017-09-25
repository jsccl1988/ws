#ifndef __SERVER_ROUTE_HPP__
#define __SERVER_ROUTE_HPP__

#include <string>
#include <wspp/util/dictionary.hpp>


namespace wspp { namespace server {

using util::Dictionary ;

struct RouteImpl ;

class Route {
public:

    // create a uri path route
    Route(const std::string &pattern) ;
    ~Route() ;

    // matches the request uri to the pattern
    // pattern is the uri pattern in the form /<pat1>/<pat2>/<pat3> ... /<patn>/
    // where each sub-pattern has the format  ({[<param>][:<regex>]}|<characters>)[?]
    // e.g. /user/{id:\d+}/{action:show|hide}/
    // If the match is succesfull the method returns true and recovers the named parameters values (e.g. id, action).
    // user ? after a sub-pattern to indicate that the element (and the subsequent elements) is optional
    // Leading and trailing slashes are ignored.

    bool matches(const std::string &path, Dictionary &data) const ;
    bool matches(const std::string &path) const ;

    // generate a url from the given route replacing parameters
    // the route should contain only na
    std::string url(const Dictionary &params, bool relative = true) const  ;

private:

    std::unique_ptr<RouteImpl> impl_ ;
};

}
}

#endif