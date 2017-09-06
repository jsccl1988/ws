#ifndef __BLOG_PAGE_CONTROLLER_HPP__
#define __BLOG_PAGE_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/util/database.hpp>
#include <wspp/util/forms.hpp>
#include <wspp/util/template_renderer.hpp>

#include "user_controller.hpp"
#include "page_view.hpp"

using wspp::sqlite::Connection ;
using wspp::Response ;
using wspp::Request ;
using wspp::Session ;
using std::string ;
using wspp::TemplateRenderer ;

class PageEditForm: public wspp::Form {
public:
    PageEditForm(Connection &con, const string &id = string()) ;

private:
    Connection &con_ ;
    string id_ ;
};


class PageController {
public:
    PageController(const Request &req, Response &resp,
                   Connection &con, UserController &user, TemplateRenderer &engine,
                   PageView &page): con_(con),
    request_(req), response_(resp), user_(user), engine_(engine), page_(page) {}

    bool dispatch() ;

    void show(const string &page_id) ;
    void list(uint pager) ;
    void create() ;
    void publish() ;
    void edit() ;
    void edit(const string &page_id) ;
    void remove() ;
    void fetch();
    void update();
protected:


private:
    Connection &con_ ;
    const Request &request_ ;
    Response &response_ ;
    UserController &user_ ;
    TemplateRenderer &engine_ ;
    PageView &page_ ;

};

#endif
