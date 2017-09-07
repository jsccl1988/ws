#ifndef __WSPP_MODELS_USER_HPP__
#define __WSPP_MODELS_USER_HPP__

#include <wspp/util/database.hpp>
#include <wspp/server/session.hpp>
#include <wspp/server/response.hpp>

namespace wspp { namespace web {

using wspp::util::sqlite::Connection ;
using wspp::server::Session ;
using wspp::server::Request ;
using wspp::server::Response ;

// a user model that handles authentication via username and password stored in database

/*
 * CREATE TABLE users ( id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE NOT NULL, password TEXT NOT NULL );
 * CREATE TABLE auth_tokens ( id INTEGER PRIMARY KEY AUTOINCREMENT, selector TEXT, token TEXT, user_id INTEGER NOT NULL, expires INTEGER );
 *
 */

class Authentication {
public:
    Authentication(const Request &req, Response &resp, Session &session, Connection &con): con_(con), session_(session), request_(req), response_(resp) {}

    void persist(const std::string &username, const std::string &id, bool remember_me = false) ;
    void forget() ;

    bool check() const ;

    std::string userName() const ;
    std::string userId() const ;

    // check database for username
    bool userNameExists(const std::string &username) ;

    // verify query password against the one stored in database
    bool verifyPassword(const std::string &query, const std::string &stored) ;

    // fetch user from database
    void load(const std::string &username, std::string &id, std::string &password) ;

protected:

    Connection &con_ ;
    Session &session_ ;
    const Request &request_ ; // used to get remember me cookie
    Response &response_ ; // used to set remember me cookie
} ;

} // namespace web
} // namespace user
#endif
