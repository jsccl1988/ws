#ifndef __ROUTES_ROUTE_CONTROLLER_HPP__
#define __ROUTES_ROUTE_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/util/database.hpp>

#include <wspp/views/forms.hpp>
#include <wspp/views/renderer.hpp>

#include <wspp/controllers/login.hpp>
#include "route_model.hpp"
#include "page_view.hpp"

using wspp::util::sqlite::Connection ;
using wspp::server::Response ;
using wspp::server::Request ;
using wspp::server::Session ;
using std::string ;
using wspp::web::TemplateRenderer ;

class RouteEditForm: public wspp::web::Form {
public:
    RouteEditForm(Connection &con, const string &id = string()) ;

private:
    wspp::web::InputField *title_field_, *slug_field_ ;
    Connection &con_ ;
    string id_ ;
};

// CREATE TABLE routes ( id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT, description TEXT, mountain TEXT )
// CREATE TABLE mountains ( id TEXT PRIMARY KEY, name TEXT, description TEXT, lat DOUBLE, lon DOUBLE )

class RouteController {
public:
    RouteController(const Request &req, Response &resp,
                   Connection &con, User &user, TemplateRenderer &engine,
                   PageView &page): routes_(con), con_(con),
    request_(req), response_(resp), user_(user), engine_(engine), page_(page) {}

    bool dispatch() ;

    void show(const string &page_id) ;
    void list(const std::string &mountain) ;
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
    RouteModel routes_ ;

};

#endif
