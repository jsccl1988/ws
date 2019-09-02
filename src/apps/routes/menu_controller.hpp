#ifndef __BLOG_MENU_CONTROLLER_HPP__
#define __BLOG_MENU_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/util/database.hpp>

#include <wspp/views/renderer.hpp>
#include <wspp/views/forms.hpp>

#include <wspp/controllers/login.hpp>

using wspp::util::sqlite::Connection;
using wspp::server::Response;
using wspp::server::Request;
using wspp::server::Session;
using std::string;
using wspp::web::TemplateRenderer;
using wspp::web::User;

class MenuForm: public wspp::web::Form {
public:
    MenuForm(Connection &con);

private:
    Connection &con_;
};

class MenuController {
public:
    MenuController(const Request &req, Response &resp,
                   Connection &con, User &user, TemplateRenderer &engine): con_(con),
    request_(req), response_(resp), user_(user), engine_(engine), menu_form_(con) {
    }

    void add();
    void edit();
    void remove();
    void fetch();
    void create();
    void update();

private:
    Connection &con_;
    const Request &request_;
    Response &response_;
    User &user_;
    TemplateRenderer &engine_;
    MenuForm menu_form_;
};
#endif
