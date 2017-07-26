#ifndef HTTP_SERVER_REQUEST_HANDLER_HPP
#define HTTP_SERVER_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/server/session.hpp>
#include <wspp/util/logger.hpp>

namespace wspp {

class ConnectionContext {
public:

    const Request &request() const { return request_ ; }
    Response &response() { return response_ ; }
    Session &session()  { return session_ ; }

protected:

    Request request_ ;
    Response response_ ;
    Session session_ ;
};

/// The common handler for all incoming requests.
class RequestHandler: private boost::noncopyable
{
public:

    explicit RequestHandler() = default;

    /// Handle a request and produce a reply. Returns true if the request was handled (e.g. the request url and method match)
    /// or not.

    virtual void handle(ConnectionContext &ctx) = 0;
};

} // namespace wspp

#endif // HTTP_SERVER_REQUEST_HANDLER_HPP
