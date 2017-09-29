#ifndef __ROUTES_ATTACHMENT_CONTROLLER_HPP__
#define __ROUTES_ATTACHMENT_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/util/database.hpp>

#include <wspp/views/forms.hpp>
#include <wspp/views/renderer.hpp>

#include "route_model.hpp"
#include "page_view.hpp"

using wspp::util::sqlite::Connection ;
using wspp::server::Response ;
using wspp::server::Request ;
using wspp::server::Session ;
using std::string ;
using wspp::web::TemplateRenderer ;

class AttachmentCreateForm: public wspp::web::Form {
public:
    AttachmentCreateForm(const Request &req, const RouteModel &routes) ;

private:
    const Request &request_ ;
    const RouteModel &routes_ ;
};

class AttachmentUpdateForm: public wspp::web::Form {
public:
    AttachmentUpdateForm(Connection &con, const RouteModel &routes) ;

private:

    Connection &con_ ;
    const RouteModel &routes_ ;
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
