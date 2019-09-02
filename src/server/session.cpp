#include <wspp/server/session.hpp>
#include <wspp/server/session_handler.hpp>

using namespace std;

namespace wspp {
namespace server {
Session::Session(SessionHandler &handler, const Request &req, Response &resp, const std::string &suffix): handler_(handler) {
    if ( handler_.open() ) {
        // check cookies and request args if session present
        string key_name = "WSX_SESSION_ID" + suffix;

        id_ = req.COOKIE_.get(key_name);
        if ( id_.empty() ) id_ = req.GET_.get(key_name);
        if ( id_.empty() ) id_ = req.POST_.get(key_name);

        if ( id_.empty() ) { // new session
            id_ = handler_.uniqueSID();
            resp.setCookie(key_name, id_, 0, handler.cookiePath(), handler.cookieDomain() );
        }
        else
            handler_.read(*this);
    }
}

Session::~Session() {
    handler_.write(*this);
    handler_.close();
}
} // namespace server
} // namesace wspp