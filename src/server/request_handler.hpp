#ifndef HTTP_SERVER_REQUEST_HANDLER_HPP
#define HTTP_SERVER_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include <memory>

namespace http {

struct Response;
struct Request;
struct SessionManager ;

/// The common handler for all incoming requests.
class RequestHandler: private boost::noncopyable
{
public:

    explicit RequestHandler() = default;

    /// Handle a request and produce a reply. Returns true if the request was handled (e.g. the request url and method match)
    /// or not.

    virtual bool handle(const Request& req, Response& rep, SessionManager &session) = 0;
};

// Use this to declare a plugin interface in a shared library
#define WSX_DECLARE_PLUGIN(class_name) extern "C" { http::RequestHandler *wsx_rh_create() { return new class_name() ; } }

} // namespace http

#endif // HTTP_SERVER_REQUEST_HANDLER_HPP
