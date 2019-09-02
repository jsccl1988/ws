#ifndef __WSPP_ROUTER_HPP__
#define __WSPP_ROUTER_HPP__

#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/server/request_handler.hpp>

#include <functional>
#include <boost/regex.hpp>

namespace wspp {
namespace server {
class Router {
public:
    Router() {}
    void addHandler();
private:
};
} // namespace server
} // namespace wspp
#endif
