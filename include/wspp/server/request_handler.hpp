#ifndef __SERVER_REQUEST_HANDLER_HPP__
#define __SERVER_REQUEST_HANDLER_HPP__

#include <string>
#include <boost/noncopyable.hpp>

#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>

namespace wspp { namespace server {

/// The common handler for all incoming requests.
class RequestHandler: private boost::noncopyable
{
public:

    explicit RequestHandler() = default;

    /// Handle a request and produce a reply. Returns true if the request was handled (e.g. the request url and method match)
    /// or not.

    virtual void handle(const Request &req, Response &resp) = 0;
};

} // namespace server
} // namespace wspp

#endif
