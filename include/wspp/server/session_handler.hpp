#ifndef __HTTP_SERVER_SESSION_HANDLER_HPP__
#define __HTTP_SERVER_SESSION_HANDLER_HPP__

#include <string>
#include <wspp/util/dictionary.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/server/session.hpp>

namespace wspp { namespace server {

class Session ;

class SessionHandler {
public:
    SessionHandler(): session_cookie_path_("/") {}

    // initialize any resources
    virtual bool open() = 0 ;

    // close and release resources
    virtual bool close() = 0 ;

    // write session data
    virtual bool write(const Session &session) = 0 ;
    // read season data
    virtual bool read(Session &session) = 0 ;
    // generate a unique SID
    virtual std::string uniqueSID() { return generateSID() ; }

    std::string cookiePath() const { return session_cookie_path_ ; }
    std::string cookieDomain() const { return session_cookie_domain_ ; }

protected:

    static std::string generateSID() ;
    std::string session_cookie_path_, session_cookie_domain_ ;
};

} // namespace server
} // namespace wspp


#endif
