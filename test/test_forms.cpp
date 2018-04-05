#include <wspp/views/forms.hpp>
#include <wspp/twig/renderer.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>

#include <memory>

using namespace std ;
using namespace wspp::twig ;
using namespace wspp::util ;
using namespace wspp::web ;

class LoginForm: public Form {
public:
    LoginForm() ;

    bool validate(const Request &vals) override ;

    void onSuccess(const Request &request) override;

private:


};

LoginForm::LoginForm() {

    field<InputField>("username", "text").required().label("Username")
        .addValidator<NonEmptyValidator>();

    field<InputField>("password", "password").required().label("Password")
        .addValidator<NonEmptyValidator>() ;

    field<InputField>("csrf_token", "hidden").initial("sklsk") ;

    field<CheckBoxField>("remember-me").label("Remember Me:") ;
}

bool LoginForm::validate(const Request &vals) {
    if ( !Form::validate(vals) ) return false ;


    string username = getValue("username") ;
    string password = getValue("password") ;

    return true ;
}

void LoginForm::onSuccess(const Request &request) {
    string username = getValue("username") ;
    bool remember_me = getValue("remember-me") == "on" ;


}

int main(int argc, char *argv[]) {

    string root = "/home/malasiot/source/ws/data/test/" ;
    std::shared_ptr<TemplateLoader> loader(new FileSystemTemplateLoader({{root}, {root + "/bootstrap3/"}})) ;
    TemplateRenderer rdr(loader) ;
    rdr.setDebug() ;

    LoginForm form ;

    form.setMethod("POST");

    cout << Variant(Variant::Object{{ "form", form.view() }}).toJSON() << endl ;

    cout << rdr.render("myform", {{ "form", form.view() }}) << endl ;


}
