#include "users_controller.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include <wspp/views/table.hpp>
#include <wspp/server/exceptions.hpp>

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;
using namespace wspp::server ;

class UserModifyForm: public wspp::web::Form {
public:
    UserModifyForm(User &user, const string &id ) ;

    void onSuccess(const Request &request) override {
        string password = getValue("password") ;
        string role = getValue("role") ;

        user_.update(id_, password, role) ;
    }
    void onGet(const Request &request) override {
    }

private:
    User &user_ ;
    string id_ ;
};

class UserCreateForm: public wspp::web::Form {
public:
    UserCreateForm(User &user) ;

    void onSuccess(const Request &request) override {
        string username = getValue("username") ;
        string password = getValue("password") ;
        string role = getValue("role") ;

        user_.create(username, password, role) ;
    }

private:
    User &user_ ;
};

UserCreateForm::UserCreateForm(User &auth): user_(auth) {

    field<InputField>("username", "text").required().label("Username")
        .setNormalizer([&] (const string &val) {
            return User::sanitizeUserName(val) ;
        })
        .addValidator<NonEmptyValidator>()
        .addValidator([&] (const string &val, const FormField &f) {
            if ( user_.userNameExists(val) )
                throw FormFieldValidationError("username already exists") ;
        }) ;

    InputField &password_field =  field<InputField>("password", "password") ;
    password_field.required().label("Password")
        .setNormalizer([] (const string &val) {
            return User::sanitizePassword(val) ;
        })
        .addValidator<NonEmptyValidator>() ;

    field<InputField>("cpassword", "password").required().label("Confirm Password")
        .setNormalizer([&] (const string &val) {
            return User::sanitizePassword(val) ;
        })
        .addValidator([&] (const string &val, const FormField &f)  {
            if ( password_field.valid() && password_field.getValue() != val  )
                throw FormFieldValidationError("Passwords don't match") ;
        });


    field<SelectField>("role", std::make_shared<DictionaryOptionsModel>(user_.auth().getRoles()))
    .required().label("Role") ;
}

UserModifyForm::UserModifyForm(User &auth, const string &id): user_(auth), id_(id) {

    InputField &password_field =  field<InputField>("password", "password") ;
    password_field.required().label("New Password")
        .setNormalizer([&] (const string &val) {
            return User::sanitizePassword(val) ;
        })
        .addValidator<NonEmptyValidator>() ;

    field<InputField>("password", "password").required().label("Confirm Password")
        .setNormalizer([&] (const string &val) {
            return User::sanitizePassword(val) ;
        })
        .addValidator([&] (const string &val, const FormField &f) {
            if ( password_field.valid() && password_field.getValue() != val  )
                throw FormFieldValidationError("Passwords don't match") ;
        }) ;

    field<SelectField>("role", std::make_shared<DictionaryOptionsModel>(user_.auth().getRoles()))
    .required().label("Role") ;
}


class UsersTableView: public SQLiteTableView {
public:
    UsersTableView(Connection &con, const Dictionary &roles): SQLiteTableView(con, "users_list_view"), roles_(roles)  {

        setTitle("Users") ;
        con_.exec("CREATE TEMPORARY VIEW users_list_view AS SELECT u.id AS id, u.name AS username, r.role_id AS role FROM users AS u JOIN user_roles AS r ON r.user_id = u.id") ;

        addColumn("Username", "{{username}}") ;
        addColumn("Role", "{{role}}") ;
    }

    Variant transform(const std::string &key, const std::string &value) override {
        if ( key == "role" ) return roles_[value] ;
        else return value ;
    }

private:
    Dictionary roles_ ;
};

void UsersController::fetch()
{
    UsersTableView view(con_, user_.auth().getRoles()) ;

    view.render(request_, response_, engine_) ;
}


void UsersController::edit()
{
    Variant ctx( Variant::Object{
        { "page", page_.data("edit_users", "Edit Users") }
    }) ;

    response_.write(engine_.render("users-edit", ctx)) ;
}



void UsersController::create() {
    UserCreateForm form(user_) ;

    form.handle(request_, response_, engine_) ;
}


void UsersController::update() {
    UserModifyForm form(user_, request_.GET_.get("id")) ;

    form.handle(request_, response_, engine_) ;
}

void UsersController::remove()
{
    const Dictionary &params = request_.POST_ ;
    string id = params.get("id") ;

    if ( id.empty() )
        throw HttpResponseException(Response::not_found) ;
    else {
        con_.execute("DELETE FROM users where id=?", id) ;
        con_.execute("DELETE FROM user_roles where user_id=?", id);
        response_.writeJSON("{}") ;
    }

}

bool UsersController::dispatch()
{
    if ( !boost::starts_with(request_.path_, "/users") ) return false ;

    Dictionary attributes ;

    bool logged_in = user_.check() ;

    if ( request_.matches("GET", "/users/edit/") ) { // load users list editor
        if ( logged_in && user_.can("users.edit")) edit() ;
        else throw HttpResponseException(Response::unauthorized) ;
    }
    if ( request_.matches("GET", "/users/list/") ) { // fetch table data
        if ( logged_in && user_.can("users.list")) fetch() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/users/add/") ) {
        if ( logged_in && user_.can("users.add")) create() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/users/update/") ) {
        if ( logged_in && user_.can("users.modify")) update() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("POST", "/users/delete/") ) {
        if ( logged_in && user_.can("users.delete") ) remove() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else
        return false ;
}




