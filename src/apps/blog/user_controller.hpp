#ifndef __BLOG_USER_CONTROLLER_HPP__
#define __BLOG_USER_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/util/database.hpp>

using wspp::sqlite::Connection ;
using wspp::Response ;
using wspp::Request ;
using wspp::Session ;
using std::string ;

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

#endif
