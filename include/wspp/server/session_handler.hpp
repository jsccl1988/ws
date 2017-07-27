#ifndef __HTTP_SERVER_SESSION_HANDLER_HPP__
#define __HTTP_SERVER_SESSION_HANDLER_HPP__

#include <string>
#include <wspp/util/dictionary.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/server/session.hpp>

namespace wspp {

class Session ;

class SessionHandler {
public:
    SessionHandler() {}

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
protected:

    static std::string generateSID() ;
};

/*
class MemSessionManager: public SessionManager {
public:
    MemSessionManager() {}

    void save(const Session &session) ;
    void load(Session &session) ;

private:
    std::map<std::string, Dictionary> data_ ;
};
*/
}


#endif
