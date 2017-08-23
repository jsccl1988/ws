#include "user_controller.hpp"

#include <wspp/util/crypto.hpp>
#include <wspp/util/variant.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

using namespace std ;
using namespace wspp ;

/*
 * CREATE TABLE users ( id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE NOT NULL, password TEXT NOT NULL );
 * CREATE TABLE auth_tokens ( id INTEGER PRIMARY KEY AUTOINCREMENT, selector TEXT, token TEXT, user_id INTEGER NOT NULL, expires INTEGER );
 *
 */

void UserController::login()
{
    const auto &params = request_.POST_ ;
    string username = params.get("username") ;
    string password = params.get("password") ;
    bool remember_me = params.get("remember-me") == "on";

    // data validation

    if ( !sanitizeUserName(username) ) {
        response_.writeJSONVariant({"error", "Invalid username"}) ;
        return ;
    }

    if ( !sanitizePassword(password) ) {
        response_.writeJSONVariant({"error", "Invalid password"}) ;
        return ;
    }

    if ( !userNameExists(username) ) {
        response_.writeJSONVariant({"error", "Username does not exist"}) ;
        return ;
    }

    string stored_password, user_id ;
    fetchUser(username, user_id, stored_password) ;

    if ( !verifyPassword(password, stored_password) ) {
        response_.writeJSONVariant({"error", "Password mismatch"}) ;
        return ;
    }

    // fill in session cache with user information

    //        session_regenerate_id(true) ;

    session_.data().add("username", username) ;
    session_.data().add("user_id", user_id) ;
    session_.data().add("role", "admin") ;

    // remove any expired tokens

    {
        sqlite::Statement stmt(con_, "DELETE FROM auth_tokens WHERE expires < ?", time(nullptr)) ;
        stmt.exec() ;
    }

    // If the user has clicked on remember me button we have to create the cookie

    if ( remember_me ) {
        string selector = encodeBase64(randomBytes(12)) ;
        string token = randomBytes(24) ;
        time_t expires 	= std::time(nullptr) + 3600*24*10; // Expire in 10 days

        sqlite::Statement stmt(con_, "INSERT INTO auth_tokens ( user_id, selector, token, expires ) VALUES ( ?, ?, ?, ? );", user_id, selector, binToHex(hashSHA256(token)), expires) ;
        stmt.exec() ;

        response_.setCookie("auth_token", selector + ":" + encodeBase64(token),  expires, "/");
    }

    // update user info
    sqlite::Statement stmt(con_, "UPDATE user_info SET last_sign_in = ? WHERE user_id = ?", std::time(nullptr), user_id) ;
    stmt.exec() ;

    response_.writeJSON("{}");
}

void UserController::logout()
{
    if ( isLoggedIn() ) {

        string user_id = session_.data().get("user_id") ;

        session_.data().remove("username") ;
        session_.data().remove("user_id") ;
        session_.data().remove("role") ;

        string cookie = request_.COOKIE_.get("auth_token") ;

        if ( !cookie.empty() ) {
            response_.setCookie("auth_token", string(), 0, "/") ;

            // update persistent tokens
            sqlite::Statement stmt(con_, "DELETE FROM auth_tokens WHERE user_id = ?", user_id) ;
            stmt.exec() ;
        }
    }
}

static bool hash_equals(const std::string &query, const std::string &stored) {

    if ( query.size() != stored.size() ) return false ;

    uint ncount = 0 ;
    for( uint i=0 ; i<stored.size() ; i++ )
        if ( query.at(i) != stored.at(i) ) ncount ++ ;

    return ncount == 0 ;
}

string UserController::name() const {
    return session_.data().get("username") ;
}

bool UserController::isLoggedIn() const
{
    if ( session_.data().contains("username") ) return true ;

    // No session information. Check if there is user info in the cookie

    string cookie = request_.COOKIE_.get("auth_token") ;

    if ( !cookie.empty() ) {

        vector<string> tokens ;
        boost::split(tokens, cookie, boost::is_any_of(":")) ;

        if ( tokens.size() == 2 && !tokens[0].empty() && !tokens[1].empty() ) {
            // if both selector and token were found check if database has the specific token

            sqlite::Query stmt(con_, "SELECT a.user_id as user_id, a.token as token, u.name as username FROM auth_tokens AS a JOIN users AS u ON a.user_id = u.id WHERE a.selector = ? AND a.expires > ? LIMIT 1",
                               tokens[0], std::time(nullptr)) ;
            sqlite::QueryResult res = stmt.exec() ;

            if ( res ) {

                string cookie_token = binToHex(hashSHA256(decodeBase64(tokens[1]))) ;
                string stored_token = res.get<string>("token") ;

                if ( hash_equals(stored_token, cookie_token) ) {
                    session_.data().add("username", res.get<string>("username")) ;
                    session_.data().add("user_id", res.get<string>("user_id")) ;
                    session_.data().add("role", "admin") ;

                    return true ;
                }
            }
        }

        // if we are here then probably there is a security issue
    }

    return false ;
}

static void strip_all_tags(string &str, bool remove_breaks = false) {
    static boost::regex rx_stags(R"(<(script|style)[^>]*?>.*?<\/\1>)", boost::regex::icase) ;
    static boost::regex rx_tags(R"(<[^>]*>)") ;
    static boost::regex rx_lb(R"([\r\n\t ]+)") ;

    // remove spacial tags and their contents
    str = boost::regex_replace(str, rx_stags, "") ;
    // remove all other tags
    str = boost::regex_replace(str, rx_tags, "") ;

    if ( remove_breaks )
        str = boost::regex_replace(str, rx_lb, " ") ;

    boost::trim(str) ;
}

bool UserController::sanitizeUserName(string &username)
{
    strip_all_tags(username) ;

    return !username.empty() ;
}

bool UserController::sanitizePassword(string &password)
{
    boost::trim(password) ;
    return !password.empty() ;
}

bool UserController::userNameExists(const string &username)
{
    sqlite::Query stmt(con_, "SELECT id FROM 'users' WHERE name = ? LIMIT 1;", username) ;
    sqlite::QueryResult res = stmt.exec() ;
    return (bool)res ;
}

bool UserController::verifyPassword(const string &query, const string &stored) {
    return passwordVerify(query, decodeBase64(stored)) ;
}

void UserController::fetchUser(const string &username, string &id, string &password)
{
    sqlite::Query stmt(con_, "SELECT id, password FROM users WHERE name = ? LIMIT 1;", username) ;
    sqlite::QueryResult res = stmt.exec() ;

    if ( res ) {
        id = res.get<string>("id") ;
        password = res.get<string>("password") ;
    }
}
