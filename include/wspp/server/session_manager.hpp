#ifndef __HTTP_SERVER_SESSION_MANAGER_HPP__
#define __HTTP_SERVER_SESSION_MANAGER_HPP__

#include <string>
#include <wspp/util/dictionary.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/server/session.hpp>

namespace wspp {

struct Session ;

class SessionManager {
public:
    SessionManager() {}

    void open(const Request &req, Session &session_data) ;
    void close(Response &resp, const Session &session_data);

protected:
    virtual void save(const Session &session) = 0 ;
    virtual void load(Session &session) = 0 ;
};


class MemSessionManager: public SessionManager {
public:
    MemSessionManager() {}

    void save(const Session &session) ;
    void load(Session &session) ;

private:
    std::map<std::string, Dictionary> data_ ;
};

}


#endif
