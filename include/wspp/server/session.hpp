#ifndef __HTTP_SERVER_SESSION_HPP__
#define __HTTP_SERVER_SESSION_HPP__

#include <string>
#include <wspp/util/dictionary.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>

namespace wspp {

class SessionManager ;

struct Session {
    std::string id_ ;
    Dictionary data_ ;
    uint64_t lifetime_ ;
};

}


#endif
