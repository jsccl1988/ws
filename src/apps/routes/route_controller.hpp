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

class RouteCreateForm: public wspp::web::Form {
public:
    RouteCreateForm(const Request &req, const RouteModel &routes) ;

    const RouteGeometry &geom() const { return geom_ ; }

private:

    RouteGeometry geom_ ;
    const Request &request_ ;
    const RouteModel &routes_ ;
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

    void view(const string &route_id) ;
    void browse(const std::string &mountain) ;
    void list() ;
    void create() ;
    void publish() ;
    void edit() ;
    void edit(const string &page_id) ;
    void remove() ;
    void fetch();
    void update();
    void uploadTrack() ;
    void track(const string &route_id) ;
    void download(const string &format, const string &route_id) ;

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
