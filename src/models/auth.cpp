#include <wspp/models/auth.hpp>
#include <wspp/util/crypto.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

using namespace std ;
using namespace wspp::util ;

namespace wspp { namespace web {

void User::persist(const std::string &user_name, const std::string &user_id, const std::string &user_role, bool remember_me)
{
    // fill in session cache with user information

    //        session_regenerate_id(true) ;

    session_.data().add("user_name", user_name) ;
    session_.data().add("user_id", user_id) ;
    session_.data().add("user_role", user_role) ;

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

}

void User::forget()
{
    if ( check() ) {

        string user_id = session_.data().get("user_id") ;

        session_.data().remove("user_name") ;
        session_.data().remove("user_id") ;
        session_.data().remove("user_role") ;

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

string User::userName() const {
    return session_.data().get("user_name") ;
}

string User::userId() const {
    return session_.data().get("user_id") ;
}

string User::userRole() const {
    return session_.data().get("user_role") ;
}

string User::token() const
{
    string session_token = session_.data().get("token") ;
    if ( session_token.empty() ) {
        string token = binToHex(randomBytes(32)) ;
        session_.data().add("token", token) ;
        return token ;
    }
    else return session_token ;
}



bool User::check() const
{
    if ( session_.data().contains("user_name") ) return true ;

    // No session information. Check if there is user info in the cookie

    string cookie = request_.COOKIE_.get("auth_token") ;

    if ( !cookie.empty() ) {

        vector<string> tokens ;
        boost::split(tokens, cookie, boost::is_any_of(":")) ;

        if ( tokens.size() == 2 && !tokens[0].empty() && !tokens[1].empty() ) {
            // if both selector and token were found check if database has the specific token

            sqlite::Query stmt(con_, "SELECT a.user_id as user_id, a.token as token, u.name as username, r.role_id as role FROM auth_tokens AS a JOIN users AS u ON a.user_id = u.id JOIN user_roles AS r ON r.user_id = u.id WHERE a.selector = ? AND a.expires > ? LIMIT 1",
                               tokens[0], std::time(nullptr)) ;
            sqlite::QueryResult res = stmt.exec() ;

            if ( res ) {

                string cookie_token = binToHex(hashSHA256(decodeBase64(tokens[1]))) ;
                string stored_token = res.get<string>("token") ;

                if ( hash_equals(stored_token, cookie_token) ) {
                    session_.data().add("user_name", res.get<string>("username")) ;
                    session_.data().add("user_id", res.get<string>("user_id")) ;
                    session_.data().add("user_role", res.get<string>("role")) ;

                    return true ;
                }
            }
        }

        // if we are here then probably there is a security issue
    }

    return false ;
}

bool User::userNameExists(const string &username)
{
    sqlite::Query stmt(con_, "SELECT id FROM 'users' WHERE name = ? LIMIT 1;", username) ;
    sqlite::QueryResult res = stmt.exec() ;
    return (bool)res ;
}

bool User::verifyPassword(const string &query, const string &stored) {
    return passwordVerify(query, decodeBase64(stored)) ;
}

void User::load(const string &username, string &id, string &password, string &role)
{
    sqlite::Query stmt(con_, "SELECT u.id AS id, u.password as password, r.role_id as role FROM users AS u JOIN user_roles AS r ON r.user_id = u.id WHERE name = ? LIMIT 1;", username) ;
    sqlite::QueryResult res = stmt.exec() ;

    if ( res ) {
        id = res.get<string>("id") ;
        password = res.get<string>("password") ;
        role = res.get<string>("role") ;
    }
}

static string glob_to_regex(const string &pat)
{
    // Convert pattern
    string rx = "^", be ;

    string::const_iterator cursor = pat.begin(), end = pat.end() ;
    bool in_char_class = false ;

    while ( cursor != end )
    {
        char c = *cursor++;

        switch (c)
        {
            case '*':
                rx += ".*?" ;
                break;
            case '$':  //Regex special characters
            case '(':
            case ')':
            case '+':
            case '.':
            case '|':
                rx += '\\';
                rx += c;
                break;
            case '\\':
                if ( *cursor == '*' ) rx += "\\*" ;
                else if ( *cursor == '?' )  rx += "\\?" ;
                cursor ++ ;
            break ;
            default:
                rx += c;
        }
    }

    rx += "$" ;
    return rx ;
}

static bool match_permissions(const string &glob, const string &action) {
    string rxs = glob_to_regex(glob) ;
    boost::regex rx(rxs) ;
    bool res = boost::regex_match(action, rx) ;
    return res ;
}

bool User::can(const string &action) const {
    string role = userRole() ;
    vector<string> permissions  = auth_.getPermissions(role) ;
    for( uint i=0 ; i<permissions.size() ; i++ ) {
        if ( match_permissions(permissions[i], action) ) return true ;
    }
    return false ;
}



static string strip_all_tags(const string &str, bool remove_breaks = false) {
    static boost::regex rx_stags(R"(<(script|style)[^>]*?>.*?<\/\1>)", boost::regex::icase) ;
    static boost::regex rx_tags(R"(<[^>]*>)") ;
    static boost::regex rx_lb(R"([\r\n\t ]+)") ;

    // remove spacial tags and their contents
    string res = boost::regex_replace(str, rx_stags, "") ;
    // remove all other tags
    res = boost::regex_replace(res, rx_tags, "") ;

    if ( remove_breaks )
        res = boost::regex_replace(res, rx_lb, " ") ;

    return boost::trim_copy(res) ;
}

string User::sanitizeUserName(const string &username)
{
    return strip_all_tags(username) ;
}

string User::sanitizePassword(const string &password)
{
    return boost::trim_copy(password) ;
}

void User::create(const string &username, const string &password, const string &role)
{
    string secure_pass = encodeBase64(passwordHash(password)) ;
    sqlite::Statement(con_, "INSERT INTO users ( name, password ) VALUES ( ?, ? )", username, secure_pass).exec() ;
    sqlite::Statement(con_, "INSERT INTO user_roles ( user_id, role_id ) VALUES ( ?, ? )", con_.last_insert_rowid(), role).exec() ;
}

void User::update(const string &id, const string &password, const string &role)
{
    string secure_pass = encodeBase64(passwordHash(password)) ;
    sqlite::Statement(con_, "UPDATE users SET password=? WHERE id=?", secure_pass, id).exec() ;
    sqlite::Statement(con_, "UPDATE user_roles SET role_id=? WHERE user_id=?", role, id).exec() ;
}

//////////////////////////////////////////////////////////////////////////////

DefaultAuthorizationModel::DefaultAuthorizationModel(Variant role_map)
{
    for( const string &key: role_map.keys() ) {
        Variant pv = role_map.at(key) ;
        Variant name = pv.at("name") ;

        string rname ;
        if ( name.isNull() ) rname = key ;
        else rname = name.toString() ;

        Variant permv = pv.at("permissions") ;
        vector<string> permissions ;
        if ( permv.isArray() ) {
            for( uint i=0 ; i<permv.length() ; i++ ) {
                Variant val = permv.at(i) ;
                if ( val.isValue() ) permissions.emplace_back(val.toString()) ;
            }
        }
        role_map_.insert({key, Role(rname, permissions)}) ;
    }
}

std::vector<string> DefaultAuthorizationModel::getPermissions(const std::string &role) const
{
    auto it = role_map_.find(role) ;
    if ( it != role_map_.end() ) return it->second.permissions_ ;
    else return {} ;
}

Dictionary DefaultAuthorizationModel::getRoles() const {
    Dictionary roles ;
    for( const auto &pr: role_map_) {
        roles.add(pr.first, pr.second.name_) ;
    }
    return roles ;
}

} // namespace web
} // namespace wspp
