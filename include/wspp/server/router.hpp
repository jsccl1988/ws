#ifndef __WSPP_ROUTER_HPP__
#define __WSPP_ROUTER_HPP__

#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/server/request_handler.hpp>
#include <wspp/server/session_manager.hpp>

#include <functional>
#include <boost/regex.hpp>

namespace wspp {

class Router: public RequestHandler {
public:
    Router(const std::string &base_path = std::string()): base_path_(base_path) {}

    // matches the request method with one of the specified methods in the list
    // matches the request uri to the pattern
    // pattern is the uri pattern in the form /<pat1>/<pat2>/<pat3> ... /<patn>/
    // where each sub-pattern has the format  <prefix>{<param>[:<verifier>]}[?]<suffix>
    // e.g. /user/{id:n}/{action:show|hide}/
    // If the match is succesfull the method returns true and recovers the named parameters values (e.g. id, action).
    // The verifier can be one of 'w' (word), 'a' (alphanumeric), 'n' (numeric), '*' (any except /) and '**' (any) or otherwise it is assumed to be
    // a verbatim regular expression (e.g. 'show|hide')

    void addRoute(const std::vector<std::string> &methods, const std::string &pattern, const std::shared_ptr<RequestHandler> &callback) {
        entries_.emplace_back(methods, pattern, callback) ;
    }

    bool handle(Request &req, Response &rep, SessionManager &session);

private:

    std::string getCleanPath(const std::string &path) ;

    static bool matchRoute(const std::vector<boost::regex> &, const std::string &uri, Dictionary &vars) ;
    static std::vector<boost::regex> makeRegexPatterns(const std::string &pat) ;

    struct Entry {
        Entry(const std::vector<std::string> &methods, const std::string &pattern, const std::shared_ptr<RequestHandler> &callback):
            methods_(methods), handler_(callback), patterns_(makeRegexPatterns(pattern)) {}

        std::shared_ptr<RequestHandler> handler_ ;
        std::vector<std::string> methods_ ;
        std::vector<boost::regex> patterns_ ;
    };

    std::vector<Entry> entries_ ;
    std::string base_path_ ;
};

}

#endif
