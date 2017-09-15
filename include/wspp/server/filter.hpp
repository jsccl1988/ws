#ifndef __SERVER_FILTER_HPP__
#define __SERVER_FILTER_HPP__

#include <string>
#include <boost/noncopyable.hpp>

#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>

namespace wspp { namespace server {

class FilterChain ;

/// Middleware handler
///
class Filter: private boost::noncopyable
{
public:

    explicit Filter() = default;

    /// Handle a request and produce a reply. Should call chain.next before or after to act as pre-post filter.

    virtual void handle(Request &req, Response &resp, FilterChain &chain) = 0;
};

} // namespace server
} // namespace wspp

#endif
