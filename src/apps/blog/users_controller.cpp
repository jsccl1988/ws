#include "users_controller.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include <wspp/views/table.hpp>

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;

UsersEditForm::UsersEditForm(User &auth, const std::string &id): user_(auth), id_(id) {

    username_field_ = boost::make_shared<InputField>("username", "text") ;
    username_field_->required() ;
    username_field_->label("Username") ;
    username_field_->setNormalizer([&] (const string &val) {
        return User::sanitizeUserName(val) ;
    }) ;
    username_field_->addValidator([&] (const string &val, FormField &f) {
        if ( val.empty() ) {
            f.addErrorMsg("Empty user name") ;
            return false ;
        }

        return true ;
    }) ;

    password_field_ = boost::make_shared<InputField>("password", "password") ;
    password_field_->required() ;
    password_field_->label("Password") ;
    password_field_->setNormalizer([&] (const string &val) {
        return User::sanitizePassword(val) ;
    }) ;
    password_field_->addValidator([&] (const string &val, FormField &f) {
        if ( val.empty() ) {
            f.addErrorMsg("Empty password") ;
            return false ;
        }

        return true ;
    }) ;

    cpassword_field_ = boost::make_shared<InputField>("password", "password") ;
    cpassword_field_->required() ;
    cpassword_field_->label("Confirm Password") ;
    cpassword_field_->setNormalizer([&] (const string &val) {
        return User::sanitizePassword(val) ;
    }) ;
    cpassword_field_->addValidator([&] (const string &val, FormField &f) {
        if ( val.empty() ) {
            f.addErrorMsg("Empty password") ;
            return false ;
        }

        return true ;
    }) ;



    role_field_ = boost::make_shared<SelectField>("role", boost::make_shared<DictionaryOptionsModel>(user_.auth().getRoles())) ;
    role_field_->required() ;

    addField(username_field_) ;
    addField(password_field_) ;
    addField(cpassword_field_) ;
    addField(role_field_) ;
}

bool UsersEditForm::validate(const Dictionary &vals) {
    /*
    if ( !Form::validate(vals) ) return false ;

    string username = getValue("username") ;
    string password = getValue("password") ;
    string cpassword = getValue("cpassword") ;

    if ( id_.empty() && user_.userNameExists(username) ) {
        errors_.push_back("Username already exists") ;
        return false ;
    }

    if ( password != cpassword ) {
        errors_.push_back("Passwords do not match") ;
        return false ;
    }
    string stored_password, user_id, role ;
    auth_.load(username, user_id, stored_password, role) ;

    if ( !auth_.verifyPassword(password, stored_password) ) {
        errors_.push_back("Password mismatch") ;
        return false ;
    }
*/
    return true ;
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
    UsersEditForm form(user_) ;

    if ( request_.method_ == "POST" ) {

        if ( form.validate(request_.POST_) ) {

            // write data to database
/*
            sqlite::Statement stmt(con_, "INSERT INTO pages ( title, permalink ) VALUES ( ?, ? )") ;

            stmt.bind(1, form.getValue("title")) ;
            stmt.bind(2, form.getValue("slug")) ;
            stmt.exec() ;
*/
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

        UsersEditForm form(user_, id) ;

        if ( form.validate(request_.POST_) ) {

            // write data to database
/*
            sqlite::Statement stmt(con_, "UPDATE pages SET title = ?, permalink = ? WHERE id = ?") ;

            stmt(form.getValue("title"), form.getValue("slug"), id) ;
*/

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

        UsersEditForm form(user_, id) ;

        if ( id.empty() ) {
            response_.stock_reply(Response::not_found) ;
            return ;
        }
/*
        sqlite::Query q(con_, "SELECT title, permalink as slug FROM pages WHERE id = ? LIMIT 1", id) ;
        sqlite::QueryResult res = q.exec() ;

        if ( !res ) {
            response_.stock_reply(Response::not_found) ;
            return ;
        }

        form.init(res.getAll()) ;
*/
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
        sqlite::Statement stmt(con_, "DELETE FROM pages where id=?", id) ;
        stmt.exec() ;
        response_.writeJSON("{}") ;
    }

}

bool UsersController::dispatch()
{
    if ( !boost::starts_with(request_.path_, "/users") ) return false ;

    Dictionary attributes ;

    bool logged_in = user_.check() ;

    if ( request_.matches("GET", "/users/edit/", attributes) ) { // load users list editor
        if ( logged_in && user_.can("users.edit")) edit() ;
        else  response_.stock_reply(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET", "/users/list/", attributes) ) { // fetch table data
        if ( logged_in && user_.can("users.list")) fetch() ;
        else  response_.stock_reply(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/users/add/", attributes) ) {
        if ( logged_in && user_.can("users.add")) create() ;
        else  response_.stock_reply(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/users/update/", attributes) ) {
        if ( logged_in && user_.can("users.modify")) update() ;
        else  response_.stock_reply(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("POST", "/users/delete") ) {
        if ( logged_in && user_.can("users.delete") ) remove() ;
        else  response_.stock_reply(Response::unauthorized) ;
        return true ;
    }
    else
        return false ;
}

void UsersController::show(const std::string &page_id)
{
    sqlite::Query q(con_, "SELECT id, title, content FROM pages WHERE permalink=?", page_id) ;
    sqlite::QueryResult res = q.exec() ;

    if ( res ) {

        Variant ctx( Variant::Object{
                     { "page", page_.data(page_id, res.get<string>("title")) },
                     { "content", res.get<string>("content") },
                     { "id", res.get<int>("id") }
        }) ;

        response_.write(engine_.render("page", ctx)) ;
    }
    else
        response_.stock_reply(Response::not_found);

}



