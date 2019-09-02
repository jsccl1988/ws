#include <wspp/server/filter_chain.hpp>
#include <wspp/server/request_handler.hpp>

namespace wspp {
namespace server {
void FilterChain::setEndPoint(RequestHandler *end_point) {
    end_point_ = end_point;
}

void FilterChain::next(Request &req, Response &resp) {
    current_++;
    doHandle(req, resp);
}

void FilterChain::add(Filter *f) {
    filters_.emplace_back(f);
}

void FilterChain::handle(Request &req, Response &resp) {
    current_ = filters_.begin();
    doHandle(req, resp);
}

void FilterChain::doHandle(Request &req, Response &resp) {
    if ( current_ != filters_.end() )
        (*current_)->handle(req, resp, *this);
    else if ( end_point_ )
        end_point_->handle(req, resp);
}
}
}
