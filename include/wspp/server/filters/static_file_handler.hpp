#ifndef __SERVER_STATIC_FILE_HANDLER_HPP__
#define __SERVER_STATIC_FILE_HANDLER_HPP__

#include <wspp/server/filter.hpp>

namespace wspp { namespace server {

class FilterChain ;
class Request ;
class Response ;

class StaticFileHandler: public Filter {
public:
    StaticFileHandler(const std::string &route_dir): root_(route_dir) {}

    void handle(Request &req, Response &resp, FilterChain &chain) override;

    std::string root_ ;
};

}
}

#endif
