#ifndef __USER_CONTROLLER_HPP__
#define __USER_CONTROLLER_HPP__

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/database/connection.hpp>
#include <wspp/views/renderer.hpp>

#include <wspp/models/auth.hpp>

using wspp::server::Response ;
using wspp::server::Request ;
using wspp::server::Session ;

using std::string ;

namespace wspp {
namespace web {


class LoginController {
public:
    LoginController(User &user, const Request &req, Response &resp, TemplateRenderer &engine):
    request_(req), response_(resp), user_(user), engine_(engine) {}

    bool dispatch() ;
    void login() ;
    void logout() ;


protected:
    // sanitize and verify username/password

    bool sanitizeUserName(string &username) ;
    bool sanitizePassword(string &password) ;

private:
    User &user_ ;
    const Request &request_ ;
    Response &response_ ;
    TemplateRenderer &engine_ ;
};

} // namespace web
} // namespace wspp
#endif
