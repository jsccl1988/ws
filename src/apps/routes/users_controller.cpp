#include "users_controller.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <boost/format.hpp>

#include <wspp/views/table.hpp>
#include <wspp/server/exceptions.hpp>

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;
using namespace wspp::server ;
using namespace wspp::db ;

class UserModifyForm: public wspp::web::FormHandler {
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

class UserCreateForm: public wspp::web::FormHandler {
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

    field("username").alias("Username")
        .setNormalizer([&] (const string &val) {
            return User::sanitizeUserName(val) ;
        })
        .addValidator<NonEmptyValidator>()
        .addValidator([&] (const string &val, const FormField &f) {
            if ( user_.userNameExists(val) )
                throw FormFieldValidationError("username already exists") ;
        }) ;

    FormField &password_field =  field("password") ;
    password_field.alias("Password")
        .setNormalizer([] (const string &val) {
            return User::sanitizePassword(val) ;
        })
        .addValidator<NonEmptyValidator>() ;

    field("cpassword")
        .setNormalizer([&] (const string &val) {
            return User::sanitizePassword(val) ;
        })
        .addValidator([&] (const string &val, const FormField &f)  {
            if ( password_field.valid() && password_field.getValue() != val  )
                throw FormFieldValidationError("Passwords don't match") ;
        });


  //  field("role", std::make_shared<DictionaryOptionsModel>(user_.auth().getRoles()))
   // .required().label("Role") ;
}

UserModifyForm::UserModifyForm(User &auth, const string &id): user_(auth), id_(id) {

    FormField &password_field =  field("password") ;
    password_field.alias("New Password")
        .setNormalizer([&] (const string &val) {
            return User::sanitizePassword(val) ;
        })
        .addValidator<NonEmptyValidator>() ;

    field("cpassword").alias("Confirm Password")
        .setNormalizer([&] (const string &val) {
            return User::sanitizePassword(val) ;
        })
        .addValidator([&] (const string &val, const FormField &f) {
            if ( password_field.valid() && password_field.getValue() != val  )
                throw FormFieldValidationError("Passwords don't match") ;
        }) ;

    field("role").alias("Role") ;//, std::make_shared<DictionaryOptionsModel>(user_.auth().getRoles()))
}


static void memoryMapDict(Connection &con, const Dictionary &dict, const string &db_id, const string &key_id, const string &val_id) {
   // con.exec(str(boost::format("ATTACH ':memory:' as %1%") % db_id)) ;
    string s = str(boost::format("CREATE TEMPORARY TABLE temp.%1% (%2% TEXT PRIMARY KEY, %3% TEXT)") % db_id % key_id % val_id) ;
    con.execute(s) ;

    Statement stmt(con, str(boost::format("INSERT INTO temp.%1% (%2%,%3%) VALUES (?, ?)") % db_id % key_id % val_id)) ;

    Transaction t(con) ;
    for( const auto &p: dict ) {
        stmt(p.first, p.second) ;
        stmt.clear() ;
    }
    t.commit() ;
}

class UsersTableView: public SQLTableView {
public:
    UsersTableView(Connection &con, const Dictionary &roles): SQLTableView(con, "users_list_view"), roles_(roles)  {

        memoryMapDict(con, roles, "user_roles_dict", "role_id", "role_label");

        con_.execute("CREATE TEMPORARY VIEW users_list_view AS SELECT u.id AS id, u.name AS username, ur.role_label AS role FROM users AS u JOIN user_roles AS r ON r.user_id = u.id JOIN temp.user_roles_dict as ur ON ur.role_id = r.role_id") ;
    }

private:
    Dictionary roles_ ;
};

void UsersController::fetch()
{
    UsersTableView view(con_, user_.auth().getRoles()) ;

    view.handle(request_, response_) ;
}


void UsersController::edit()
{
    Variant::Object ctx{
        { "page", page_.data("edit_users", "Edit Users") }
    };

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
        return true ;
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




