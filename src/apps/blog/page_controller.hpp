#ifndef __BLOG_PAGE_CONTROLLER_HPP__
#define __BLOG_PAGE_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/util/database.hpp>
#include <wspp/util/template_renderer.hpp>

#include "user_controller.hpp"

using wspp::sqlite::Connection ;
using wspp::Response ;
using wspp::Request ;
using wspp::Session ;
using std::string ;
using wspp::TemplateRenderer ;

class PageController {
public:
    PageController(const Request &req, Response &resp,
                   Connection &con, UserController &user, TemplateRenderer &engine): con_(con),
    request_(req), response_(resp), user_(user), engine_(engine) {}

    void show(const string &page_id) ;
    void list(uint pager) ;
    void create() ;
    void publish() ;
    void edit(const string &page_id) ;
    void remove() ;


protected:


private:
    Connection &con_ ;
    const Request &request_ ;
    Response &response_ ;
    UserController &user_ ;
    TemplateRenderer &engine_ ;
};

#endif
