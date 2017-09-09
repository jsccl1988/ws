#ifndef __BLOG_PAGE_CONTROLLER_HPP__
#define __BLOG_PAGE_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/util/database.hpp>

#include <wspp/views/forms.hpp>
#include <wspp/views/renderer.hpp>

#include <wspp/controllers/login.hpp>
#include "page_view.hpp"

using wspp::util::sqlite::Connection ;
using wspp::server::Response ;
using wspp::server::Request ;
using wspp::server::Session ;
using std::string ;
using wspp::web::TemplateRenderer ;

class PageEditForm: public wspp::web::Form {
public:
    PageEditForm(Connection &con, const string &id = string()) ;

private:
    boost::shared_ptr<wspp::web::InputField> title_field_, slug_field_ ;
    Connection &con_ ;
    string id_ ;
};


class PageController {
public:
    PageController(const Request &req, Response &resp,
                   Connection &con, User &user, TemplateRenderer &engine,
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
    User &user_ ;
    TemplateRenderer &engine_ ;
    PageView &page_ ;

};

#endif
