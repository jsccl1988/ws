#ifndef __SERVER_FILTER_CHAIN_HPP__
#define __SERVER_FILTER_CHAIN_HPP__

#include <vector>
#include <memory>

#include <wspp/server/filter.hpp>

namespace wspp {
namespace server {
class Filter;
class Request;
class Response;
class RequestHandler;
class FilterChain {
public:
    FilterChain() = default;

    void setEndPoint(RequestHandler *end_point);
    void next(Request &req, Response &resp);
    void add(Filter *);
    void handle(Request &req, Response &resp);

private:
    void doHandle(Request &req, Response &resp);
    typedef std::vector<std::unique_ptr<Filter>> FilterList;

    FilterList filters_;
    FilterList::const_iterator current_;
    RequestHandler *end_point_;
};
}
}
#endif
