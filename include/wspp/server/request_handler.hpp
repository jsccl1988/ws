#ifndef HTTP_SERVER_REQUEST_HANDLER_HPP
#define HTTP_SERVER_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/server/session.hpp>

namespace wspp {

/// The common handler for all incoming requests.
class RequestHandler: private boost::noncopyable
{
public:

    explicit RequestHandler() = default;

    /// Handle a request and produce a reply. Returns true if the request was handled (e.g. the request url and method match)
    /// or not.

    virtual void handle(const Request& req, Response& rep, Session &session) = 0;
};

} // namespace wspp

#endif // HTTP_SERVER_REQUEST_HANDLER_HPP
