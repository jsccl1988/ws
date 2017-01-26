#include "session.hpp"
#include "session_manager.hpp"

namespace http {

Session::Session(SessionManager &sm, const Request &req, Response &resp):
    session_manager_(sm), response_(resp) {
    sm.begin(req, *this) ;
}

Session::~Session() {
    session_manager_.end(response_, *this) ;
}

}
