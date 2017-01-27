#ifndef __HTTP_SERVER_SESSION_HPP__
#define __HTTP_SERVER_SESSION_HPP__

#include <string>
#include "util/dictionary.hpp"
#include "server/request.hpp"
#include "server/reply.hpp"

namespace http {

class SessionManager ;

struct Session {

    std::string id_ ;
    Dictionary data_ ;
    uint64_t lifetime_ ;

};

}


#endif
