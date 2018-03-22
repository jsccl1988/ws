#ifndef __ROUTES_ATTACHMENT_CONTROLLER_HPP__
#define __ROUTES_ATTACHMENT_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/database/connection.hpp>

#include <wspp/views/forms.hpp>
#include <wspp/views/renderer.hpp>

#include "route_model.hpp"
#include "page_view.hpp"

using wspp::db::Connection ;
using wspp::server::Response ;
using wspp::server::Request ;
using wspp::server::Session ;
using std::string ;
using wspp::web::TemplateRenderer ;

class AttachmentCreateForm: public wspp::web::Form {
public:
    AttachmentCreateForm(const Request &req, RouteModel &routes, const std::string &route_id, const string &upload_folder) ;

    void onSuccess(const Request &request) override ;

private:
    const Request &request_ ;
    RouteModel &routes_ ;
    const string upload_folder_ ;
    const string route_id_ ;
};

class AttachmentUpdateForm: public wspp::web::Form {
public:
    AttachmentUpdateForm(RouteModel &routes, const string &route_id) ;

    void onSuccess(const Request &request) override ;
    void onGet(const Request &request) override ;
private:

    RouteModel &routes_ ;
    string route_id_ ;
};

class AttachmentController {
public:
    AttachmentController(const Request &req, Response &resp,
                   Connection &con, User &user, TemplateRenderer &engine, const std::string &upload_folder): routes_(con), con_(con),
    request_(req), response_(resp), user_(user), engine_(engine), upload_folder_(upload_folder) {}

    bool dispatch() ;

    void list(const std::string &route_id) ;
    void create(const std::string &route_id) ;
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
