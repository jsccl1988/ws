#ifndef __SERVER_EXCEPTIONS_HPP__
#define __SERVER_EXCEPTIONS_HPP__

#include <wspp/server/response.hpp>

namespace wspp {
namespace server {
class HttpResponseException {
public:
    HttpResponseException(Response::Status code, const std::string &reason = std::string()):
        code_(code), reason_(reason) {}

    Response::Status code_;
    std::string reason_;
};
}
}
#endif
