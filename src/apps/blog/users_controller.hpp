#ifndef __USERS_CONTROLLER_HPP__
#define __USERS_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/util/database.hpp>

#include <wspp/views/forms.hpp>
#include <wspp/views/renderer.hpp>

#include <wspp/models/auth.hpp>
#include "page_view.hpp"

using wspp::util::sqlite::Connection ;
using wspp::util::Dictionary ;
using wspp::server::Response ;
using wspp::server::Request ;
using wspp::server::Session ;
using std::string ;
using wspp::web::TemplateRenderer ;
using wspp::web::User ;

class UsersEditForm: public wspp::web::Form {
public:
    UsersEditForm(User &user,  const string &id = string()) ;

    bool validate(const Dictionary &vals) override ;

private:
    User &user_ ;
    boost::shared_ptr<wspp::web::InputField> username_field_, password_field_, cpassword_field_ ;
    boost::shared_ptr<wspp::web::SelectField>  role_field_ ;
    string id_ ;
};


class UsersController {
public:
    UsersController(const Request &req, Response &resp,
                   Connection &con,
                   User &user, TemplateRenderer &engine,
                   PageView &page):
    request_(req), response_(resp), user_(user), engine_(engine), page_(page), con_(con) {}

    bool dispatch() ;

    void show(const string &user_id) ;
    void list(uint pager) ;
    void create() ;
    void publish() ;
    void edit() ;
    void edit(const string &user_id) ;
    void remove() ;
    void fetch();
    void update();
protected:


private:

    const Request &request_ ;
    Response &response_ ;
    User &user_ ;
    TemplateRenderer &engine_ ;
    PageView &page_ ;
    Connection &con_ ;

};

#endif






