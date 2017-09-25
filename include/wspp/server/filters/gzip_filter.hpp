#ifndef __SERVER_GZIP_FILTER_HPP__
#define __SERVER_GZIP_FILTER_HPP__

#include <wspp/server/filter.hpp>

namespace wspp { namespace server {

class FilterChain ;
class Request ;
class Response ;

class GZipFilter: public Filter {
public:
    GZipFilter() {}

    void handle(Request &req, Response &resp, FilterChain &chain) override;
};

}
}

#endif
