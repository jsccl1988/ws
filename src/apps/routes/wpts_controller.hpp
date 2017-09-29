#ifndef __ROUTES_WAYPOINTS_CONTROLLER_HPP__
#define __ROUTES_WAYPOINTS_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/util/database.hpp>

#include <wspp/views/forms.hpp>
#include <wspp/views/renderer.hpp>
#include <wspp/util/variant.hpp>

#include <wspp/models/auth.hpp>

using wspp::web::User ;
using wspp::util::Variant ;

#include "route_model.hpp"

using wspp::util::sqlite::Connection ;
using wspp::server::Response ;
using wspp::server::Request ;
using wspp::server::Session ;
using std::string ;
using wspp::web::TemplateRenderer ;


class WaypointUpdateForm: public wspp::web::Form {
public:
    WaypointUpdateForm(Connection &con, const RouteModel &routes) ;

private:

    Connection &con_ ;
    const RouteModel &routes_ ;
};

class WaypointController {
public:
    WaypointController(const Request &req, Response &resp,
                   Connection &con, User &user, TemplateRenderer &engine): routes_(con), con_(con),
    request_(req), response_(resp), user_(user), engine_(engine) {}

    bool dispatch() ;

    void list(const std::string &route_id) ;

    void remove(const std::string &route_id) ;
    void update(const std::string &route_id);

private:
    Connection &con_ ;
    const Request &request_ ;
    Response &response_ ;
    User &user_ ;
    TemplateRenderer &engine_ ;
    RouteModel routes_ ;
    std::string upload_folder_ ;

};

#endif
