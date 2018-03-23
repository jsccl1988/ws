#ifndef __USERS_CONTROLLER_HPP__
#define __USERS_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/database/connection.hpp>

#include <wspp/views/forms.hpp>
#include <wspp/views/renderer.hpp>

#include "auth.hpp"
#include "page_view.hpp"

using wspp::db::Connection ;
using wspp::util::Dictionary ;
using wspp::server::Response ;
using wspp::server::Request ;
using wspp::server::Session ;
using std::string ;
using wspp::web::TemplateRenderer ;

class UsersController {
public:
    UsersController(const Request &req, Response &resp,
                   Connection &con,
                   User &user, TemplateRenderer &engine,
                   PageView &page):
    request_(req), response_(resp), user_(user), engine_(engine), page_(page), con_(con) {}

    bool dispatch() ;

    void create() ;

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






