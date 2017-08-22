#include "user_controller.hpp"

#include <wspp/util/crypto.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

using namespace std ;
using namespace wspp ;

void UserController::login()
{
    const auto &params = request_.POST_ ;
    string username = params.get("username") ;
    string password = params.get("password") ;
    bool remember_me = params.get("remember-me") == "on";

    // data validation

    if ( !sanitizeUserName(username) ) {
        response_.writeJSON(R"({"username", "Invalid username"})") ;
        return ;
    }

    if ( !sanitizePassword(password) ) {
        response_.writeJSON(R"({"password", "Invalid password"})") ;
        return ;
    }

    if ( !userNameExists(username) ) {
        response_.writeJSON(R"({"username", "Username does not exist"})") ;
        return ;
    }

    string stored_password, user_id ;
    fetchUser(username, user_id, stored_password) ;

    if ( !verifyPassword(password, stored_password) ) {
        response_.writeJSON(R"({"password", "Password mismatch"})") ;
        return ;
    }

    // fill in session cache with user information

    //        session_regenerate_id(true) ;

    session_.data().add("username", username) ;
    session_.data().add("user_id", user_id) ;
    session_.data().add("role", "admin") ;

    // If the user has clicked on remember me button we have to create the cookie

    if ( remember_me ) {
        string selector = binToHex(randomBytes(24)) ;
        string token = binToHex(randomBytes(32)) ;
        time_t expires 	= std::time(nullptr) + 3600*24*10; // Expire in 10 days

        sqlite::Statement stmt(con_, "INSERT INTO auth_tokens ( userid, selector, token, expires ) VALUES ( ?, ?, ?, ? );", user_id, selector, token, expires) ;
        stmt.exec() ;

        //  setcookie('user_token', $selector . '.' . $token,  $expires);
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

        /*
                if ( isset($_COOKIE['user_token']) ) {

                    $stmt = $this->db_->prepare("DELETE FROM auth_tokens WHERE userid = ?;");
                    $stmt->execute(array($userid)) ;

                    setcookie('user_token','',time()-3600) ;
                }
          */
        session_.data().remove("username") ;
        session_.data().remove("user_id") ;
        session_.data().remove("role") ;
    }
}

bool UserController::isLoggedIn()
{
    if ( session_.data().contains("username") ) return true ;

    // No session information. Check if there is user info in the cookie
/*
    if( isset($_COOKIE['user_token']) )
    {
        $user_token = $_COOKIE['user_token'] ;

        $subtokens = explode('.', $_COOKIE['user_token'], 2);
            // if both selector and token were found
        if ( isset($subtokens[0]) && isset($subtokens[1]) ) {
            $stmt = $this->db_->prepare("SELECT a.userid, a.token, a.expires, u.username, u.role FROM auth_tokens AS a JOIN users AS u ON a.userid = u.id WHERE a.selector = ? LIMIT 1");
            $stmt->execute(array($subtokens[0])) ;
            $data = $stmt->fetch(\PDO::FETCH_ASSOC);

            if ( !empty($data) ) {
                if ($data['expires'] >= time() ) {
                    if ( $subtokens[1] == $data['token'] ) {
                            $_SESSION["username"] = $data["username"] ;
                            $_SESSION["userid"] = $data["userid"] ;
                            $_SESSION["role"] = $data["role"] ;
                            return true ;
                    }
                }
            }
        }
    }
*/
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
    try {
    sqlite::Query stmt(con_, "SELECT id FROM 'users' WHERE name = ? LIMIT 1;", username) ;
    sqlite::QueryResult res = stmt.exec() ;
     return (bool)res ;
    }
    catch ( sqlite::Exception &e ) {
        cout << e.what() << endl;
    }


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
