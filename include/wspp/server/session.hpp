#ifndef __HTTP_SERVER_SESSION_HPP__
#define __HTTP_SERVER_SESSION_HPP__

#include <string>
#include <wspp/util/dictionary.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>

namespace wspp {

class SessionHandler ;

class Session {
public:
    // start a new session
    Session(SessionHandler &handler, const Request &req, Response &resp, const std::string &suffix = std::string()) ;

    // closes the season
    ~Session() ;

    std::string id() const { return id_ ; }

    Dictionary &data() { return data_ ; }
    const Dictionary &data() const { return data_ ; }

private:

    std::string id_ ;
    Dictionary data_ ;
    uint64_t lifetime_ ;
    SessionHandler &handler_ ;
};

}


#endif
