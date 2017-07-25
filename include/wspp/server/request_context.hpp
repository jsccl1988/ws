#ifndef __WSPP_REQUEST_CONTEXT_HPP__
#define __WSPP_REQUEST_CONTEXT_HPP__

#include <wspp/server/session_manager.hpp>

namespace wspp {

class RequestContext {
public:
    RequestContext(SessionManager &sm) ;

private:

    SessionManager &session_manager_ ;
} ;

}

#endif
