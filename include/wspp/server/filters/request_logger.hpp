#ifndef __SERVER_REQUEST_LOGGER_HPP__
#define __SERVER_REQUEST_LOGGER_HPP__

#include <wspp/util/logger.hpp>
#include <wspp/server/filter.hpp>

namespace wspp { namespace server {

class FilterChain ;
class Request ;
class Response ;

using wspp::util::Logger ;

class RequestLoggerFilter: public Filter {
public:
    RequestLoggerFilter(Logger &logger): logger_(logger) {}

    void handle(Request &req, Response &resp, FilterChain &chain) override;

    Logger &logger_ ;
};

}
}

#endif
