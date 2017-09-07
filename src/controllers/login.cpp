#include <wspp/controllers/login.hpp>

#include <wspp/util/crypto.hpp>
#include <wspp/util/variant.hpp>

#include <wspp/views/forms.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

using namespace std ;
using namespace wspp::util ;


namespace wspp { namespace web {

class LoginForm: public wspp::web::Form {
public:
    LoginForm(Authentication &auth) ;

    string sanitizeUserName(const string &username);
    string sanitizePassword(const string &password);

    bool validate(const Dictionary &vals) override ;
private:
    Authentication &auth_ ;
    string username_ ;
};



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

string LoginForm::sanitizeUserName(const string &username)
{
    return strip_all_tags(username) ;
}

string LoginForm::sanitizePassword(const string &password)
{
    return boost::trim_copy(password) ;
}

LoginForm::LoginForm(Authentication &auth): auth_(auth) {

    input("username", "text").label("User Name:").required()
            .setNormalizer([&] (const string &val) {
                return sanitizeUserName(val) ;
            })
            .addValidator([&] (const string &val, FormField &f) {
                if ( val.empty() ) {
                    f.addErrorMsg("Empty user name") ;
                    return false ;
                }

                return true ;
            }
    ) ;

    input("password", "password").label("Password:").required()
            .setNormalizer([&](const string &val) {
                return sanitizePassword(val) ;
            })

            .addValidator([&] (const string &val, FormField &f) {
                if ( val.empty() ) {
                    f.addErrorMsg("Empty password") ;
                    return false ;
                }

                return true ;
            }
    ) ;

    checkbox("remember-me").label("Remember Me:") ;
}

bool LoginForm::validate(const Dictionary &vals) {
    if ( !Form::validate(vals) ) return false ;

    string username = getValue("username") ;
    string password = getValue("password") ;

    if ( !auth_.userNameExists(username) ) {
        errors_.push_back("Username does not exist") ;
        return false ;
    }

    string stored_password, user_id ;
    auth_.load(username, user_id, stored_password) ;

    if ( !auth_.verifyPassword(password, stored_password) ) {
        errors_.push_back("Password mismatch") ;
        return false ;
    }

    return true ;
}

bool LoginController::dispatch()
{
    if ( request_.matches("GET|POST", "/user/login/") ) login() ;
    else if ( request_.matches("POST", "/user/logout/") ) logout() ;
    else return false ;
    return true ;
}

void LoginController::login()
{
    LoginForm form(user_) ;

    if ( request_.method_ == "POST" ) {

        if ( form.validate(request_.POST_) ) {

            string username = form.getValue("username") ;
            bool remember_me = form.getValue("remember-me") == "on" ;

            string stored_password, user_id ;
            user_.load(username, user_id, stored_password) ;
            user_.persist(username, user_id, remember_me) ;

            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}}) ;
        }
        else {
            Variant ctx( Variant::Object{{"form", form.data()}} ) ;

            cout << ctx.toJSON() << endl ;
            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("login-dialog", ctx)}});
        }
    }
    else {
        Variant ctx( Variant::Object{{"form", form.data()}} ) ;

        response_.write(engine_.render("login-dialog", ctx)) ;
    }
}

void LoginController::logout()
{
    user_.forget() ;
    response_.writeJSON("{}");
}



} // namespace web
} // namespace wspp
