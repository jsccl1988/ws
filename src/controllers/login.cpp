#include <wspp/controllers/login.hpp>

#include <wspp/util/crypto.hpp>
#include <wspp/util/variant.hpp>

#include <wspp/views/forms.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/make_shared.hpp>

using namespace std ;
using namespace wspp::util ;


namespace wspp { namespace web {

class LoginForm: public wspp::web::Form {
public:
    LoginForm(User &auth) ;

    bool validate(const Dictionary &vals) override ;
private:
    User &auth_ ;
    string username_ ;
    InputField *username_field_, *password_field_ ;
    CheckBoxField *rememberme_field_ ;
};



LoginForm::LoginForm(User &auth): auth_(auth) {

    field<InputField>("username", "text").required().label("Username")
        .setNormalizer([&] (const string &val) {
            return User::sanitizeUserName(val) ;
        })
        .addValidator([&] (const string &val, FormField &f) {
            f.validateNonEmptyField(val, "User name" ) ;
        }) ;

    field<InputField>("password", "password").required().label("Password")
        .setNormalizer([&] (const string &val) {
            return User::sanitizePassword(val) ;
        })
        .addValidator([&] (const string &val, FormField &f) {
            if ( val.empty() ) {
                f.validateNonEmptyField(val, "User name" ) ;
                f.addErrorMsg("Empty password") ;
                return false ;
            }

            return true ;
        }) ;

    field<CheckBoxField>("remember-me").label("Remember Me:") ;

}

bool LoginForm::validate(const Dictionary &vals) {
    if ( !Form::validate(vals) ) return false ;

    string username = getValue("username") ;
    string password = getValue("password") ;

    if ( !auth_.userNameExists(username) ) {
        errors_.push_back("Username does not exist") ;
        return false ;
    }

    string stored_password, user_id, role ;
    auth_.load(username, user_id, stored_password, role) ;

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

            string stored_password, user_id, role ;
            user_.load(username, user_id, stored_password, role) ;
            user_.persist(username, user_id, role, remember_me) ;

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
