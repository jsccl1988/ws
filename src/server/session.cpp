#include <wspp/server/session.hpp>
#include <wspp/server/session_handler.hpp>

using namespace std ;

namespace wspp {

Session::Session(SessionHandler &handler, const Request &req, Response &resp, const std::string &suffix): handler_(handler) {
    if ( handler_.open() ) {

        // check cookies and request args if session present

        string key_name = "WSX_SESSION_ID" + suffix ;

        id_ = req.COOKIE_.get(key_name) ;
        if ( id_.empty() ) id_ = req.GET_.get(key_name) ;
        if ( id_.empty() ) id_ = req.POST_.get(key_name) ;

        if ( id_.empty() ) { // new session
            id_ = handler_.uniqueSID() ;
            resp.headers_.add("Set-Cookie", key_name + "=" + id_) ;
        }
        else
            handler_.read(*this) ;
    }
}

Session::~Session() {
    handler_.write(*this) ;
    handler_.close() ;
}

}
