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

class PageCreateForm: public wspp::web::Form {
public:
    PageCreateForm(Connection &con) ;

    void onSuccess(const Request &request) override ;

private:
    Connection &con_ ;
};

class PageUpdateForm: public wspp::web::Form {
public:
    PageUpdateForm(Connection &con, const std::string &id) ;

    void onSuccess(const Request &request) override ;
    void onGet(const Request &request) override ;

private:
    Connection &con_ ;
    std::string id_ ;
};

class PageController {
public:
    PageController(const Request &req, Response &resp,
                   Connection &con, User &user, TemplateRenderer &engine,
                   PageView &page): con_(con),
    request_(req), response_(resp), user_(user), engine_(engine), page_(page) {}

    bool dispatch() ;

    void show(const string &page_id) ;

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
