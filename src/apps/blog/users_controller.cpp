#include "users_controller.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include <wspp/views/table.hpp>

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;

class UserModifyForm: public wspp::web::Form {
public:
    UserModifyForm(User &user,  const string &id ) ;

private:
    User &user_ ;
    string id_ ;
};

class UserCreateForm: public wspp::web::Form {
public:
    UserCreateForm(User &user) ;

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


    field<SelectField>("role", boost::make_shared<DictionaryOptionsModel>(user_.auth().getRoles()))
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

    field<SelectField>("role", boost::make_shared<DictionaryOptionsModel>(user_.auth().getRoles()))
    .required().label("Role") ;
}


class UsersTableView: public SQLiteTableView {
public:
    UsersTableView(Connection &con, const Dictionary &roles): SQLiteTableView(con, "users_list_view"), roles_(roles)  {

        con_.exec("CREATE TEMPORARY VIEW users_list_view AS SELECT u.id AS id, u.name AS username, r.role_id AS role FROM users AS u JOIN user_roles AS r ON r.user_id = u.id") ;

        addColumn("Username", "username") ;
        addColumn("Role", "role") ;
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
    uint offset = request_.GET_.value<int>("page", 1) ;
    uint results_per_page = request_.GET_.value<int>("total", 10) ;

    Variant data = view.fetch(offset, results_per_page ) ;

    response_.write(engine_.render("users-table-view", data )) ;
}


void UsersController::edit()
{
    Variant ctx( Variant::Object{
        { "page", page_.data("edit_users", "Edit Users") }
    }) ;

    response_.write(engine_.render("users-edit", ctx)) ;
}



void UsersController::create()
{
    UserCreateForm form(user_) ;

    if ( request_.method_ == "POST" ) {

        if ( form.validate(request_.POST_) ) {

            // write data to database

            string username = form.getValue("username") ;
            string password = form.getValue("password") ;
            string role = form.getValue("role") ;

            user_.create(username, password, role) ;

            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}}) ;
        }
        else {
            Variant ctx( Variant::Object{{"form", form.data()}} ) ;

            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("users-edit-dialog-new", ctx)}});
        }
    }
    else {
        Variant ctx( Variant::Object{{"form", form.data()}} ) ;

        response_.write(engine_.render("users-edit-dialog-new", ctx)) ;
    }

}


void UsersController::update()
{
    if ( request_.method_ == "POST" ) {

        string id = request_.POST_.get("id") ;

        UserModifyForm form(user_, id) ;

        if ( form.validate(request_.POST_) ) {

            string password = form.getValue("password") ;
            string role = form.getValue("role") ;

            user_.update(id, password, role) ;

            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}}) ;
        }
        else {
            Variant ctx( Variant::Object{{"form", form.data()}} ) ;

            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("users-edit-dialog-new", ctx)}});
        }
    }
    else {

        const Dictionary &params = request_.GET_ ;
        string id = params.get("id") ;

        UserModifyForm form(user_, id) ;

        if ( id.empty() ) {
            response_.stock_reply(Response::not_found) ;
            return ;
        }

        Variant ctx( Variant::Object{{"form", form.data()}} ) ;

        response_.write(engine_.render("users-edit-dialog-new", ctx)) ;
    }

}

void UsersController::remove()
{
   const Dictionary &params = request_.POST_ ;
    string id = params.get("id") ;

    if ( id.empty() )
        response_.stock_reply(Response::not_found) ;
    else {
        sqlite::Statement(con_, "DELETE FROM users where id=?", id).exec() ;
        sqlite::Statement(con_, "DELETE FROM user_roles where user_id=?", id).exec() ;
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
        else  response_.stock_reply(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET", "/users/list/") ) { // fetch table data
        if ( logged_in && user_.can("users.list")) fetch() ;
        else  response_.stock_reply(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/users/add/") ) {
        if ( logged_in && user_.can("users.add")) create() ;
        else  response_.stock_reply(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/users/update/") ) {
        if ( logged_in && user_.can("users.modify")) update() ;
        else  response_.stock_reply(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("POST", "/users/delete/") ) {
        if ( logged_in && user_.can("users.delete") ) remove() ;
        else  response_.stock_reply(Response::unauthorized) ;
        return true ;
    }
    else
        return false ;
}




