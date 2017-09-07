#include <string>
#include <sstream>
#include <iostream>

#include <wspp/server/request_handler.hpp>
#include <wspp/server/response.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/fs_session_handler.hpp>
#include <wspp/server/session.hpp>
#include <wspp/server/server.hpp>

#include <wspp/util/logger.hpp>
#include <wspp/util/database.hpp>
#include <wspp/util/crypto.hpp>
#include <wspp/util/variant.hpp>

#include <wspp/views/renderer.hpp>
#include <wspp/views/menu.hpp>

#include <boost/make_shared.hpp>

#include <iostream>

#include <wspp/controllers/user_controller.hpp>

#include "page_controller.hpp"
#include "menu_controller.hpp"

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;
using namespace wspp::server ;

class DefaultLogger: public Logger
{
public:
    DefaultLogger(const std::string &log_file, bool debug) {
        if ( debug ) addAppender(std::make_shared<LogStreamAppender>(Trace, make_shared<LogPatternFormatter>("%In function %c, %F:%l: %m"), std::cerr)) ;
        addAppender(std::make_shared<LogFileAppender>(Info, make_shared<LogPatternFormatter>("%V [%d{%c}]: %m"), log_file)) ;
    }
};



class MyServer: public Server {

public:
    MyServer(const std::string &port,
             const std::string &root_dir,
             const std::string &logger_dir):
        logger_(logger_dir, true), root_(root_dir), Server("127.0.0.1", port),
        engine_(boost::shared_ptr<TemplateLoader>(new FileSystemTemplateLoader({{root_ + "/templates/"}, {root_ + "/templates/bootstrap-partials/"}})))
    {

    }

    void log(const Request &req, Response &resp) {

        LOG_X_STREAM(logger_, Info, "Response to " <<
                     getIPAddress()
                       << ": \"" << req.method_ << " " << req.path_
                       << ((req.query_.empty()) ? "" : "?" + req.query_) << " "
                       << req.protocol_ << "\" "
                       << resp.status_ << " " << resp.headers_.value<int>("Content-Length", 0)
                     ) ;

    }

    void handle(const Request &req, Response &resp) override {

        sqlite::Connection con(root_ + "/db.sqlite") ;
        Session session(sm_, req, resp) ;

        UserController user(req, resp, con, session) ;

        PageView page(user) ;

        // request router

        Dictionary attributes ;

        if ( PageController(req, resp, con, user, engine_, page).dispatch() ) return ;
        else if ( req.matches("GET", "/menu/edit/") ) {
            MenuController menus(req, resp, con, user, engine_) ;
            menus.edit() ;
        }
        else if ( req.matches("GET|POST", "/menu/create/") ) {
            MenuController menus(req, resp, con, user, engine_) ;
            menus.create() ;
        }
        else if ( req.matches("GET|POST", "/menu/update/") ) {
            MenuController menus(req, resp, con, user, engine_) ;
            menus.update() ;
        }
        else if ( req.matches("POST", "/menu/delete/") ) {
            MenuController menus(req, resp, con, user, engine_) ;
            menus.remove() ;
        }
        else if ( req.matches("GET", "/menu/list/") ) {
            MenuController menus(req, resp, con, user, engine_) ;
            menus.fetch() ;
        }
        else if ( req.matches("POST", "/user/login/") ) user.login() ;
        else if ( req.matches("POST", "/user/logout/") ) user.logout() ;
        else if ( req.matches("GET", "/{fpath:**}", attributes) ) {
            string fpath = attributes.get("fpath") ;
            resp.encode_file(root_ + fpath);
        }
        else resp.stock_reply(Response::not_found) ;

        log(req, resp) ;
    }


    FileSystemSessionHandler sm_ ;
    DefaultLogger logger_ ;
    string root_ ;
    TemplateRenderer engine_ ;

};


int main(int argc, char *argv[]) {

    MyServer server( "5000", "/home/malasiot/source/ws/data/blog/", "/tmp/logger") ;
    server.run() ;
}
