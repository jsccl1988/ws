#ifndef __USER_CONTROLLER_HPP__
#define __USER_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/util/database.hpp>

using wspp::server::Response ;
using wspp::server::Request ;
using wspp::server::Session ;

using std::string ;

namespace wspp {
namespace web {

using util::sqlite::Connection ;

class UserController {
public:
    UserController(const Request &req, Response &resp,
                   Connection &con, Session &session): con_(con), session_(session),
    request_(req), response_(resp) {}

    void login() ;
    void logout() ;

    bool isLoggedIn() const ;
    std::string name() const ;

protected:
    // sanitize and verify username/password
    bool sanitizeUserName(string &username) ;
    bool sanitizePassword(string &password) ;
    // check database for username
    bool userNameExists(const string &username) ;

    // verify query password against the one stored in database
    bool verifyPassword(const string &query, const string &stored) ;

    // fetch user from database
    void fetchUser(const string &username, string &id, string &password) ;

private:
    Connection &con_ ;
    Session &session_ ;
    const Request &request_ ;
    Response &response_ ;
};

} // namespace web
} // namespace wspp
#endif
