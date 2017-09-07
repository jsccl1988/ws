#ifndef __BLOG_MENU_CONTROLLER_HPP__
#define __BLOG_MENU_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/util/database.hpp>

#include <wspp/views/renderer.hpp>
#include <wspp/views/forms.hpp>

#include <wspp/controllers/user_controller.hpp>

using wspp::util::sqlite::Connection ;
using wspp::server::Response ;
using wspp::server::Request ;
using wspp::server::Session ;
using std::string ;
using wspp::web::TemplateRenderer ;
using wspp::web::UserController ;

class MenuForm: public wspp::web::Form {
public:
    MenuForm(Connection &con) ;

private:
    Connection &con_ ;
};

class MenuController {
public:
    MenuController(const Request &req, Response &resp,
                   Connection &con, UserController &user, TemplateRenderer &engine): con_(con),
    request_(req), response_(resp), user_(user), engine_(engine), menu_form_(con) {

    }

    void add() ;
    void edit() ;
    void remove() ;
    void fetch() ;
    void create() ;
    void update() ;

private:
    Connection &con_ ;
    const Request &request_ ;
    Response &response_ ;
    UserController &user_ ;
    TemplateRenderer &engine_ ;
    MenuForm menu_form_ ;

};

#endif
