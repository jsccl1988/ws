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
#include <wspp/util/template_renderer.hpp>

#include <iostream>

#include "user_controller.hpp"

using namespace std ;
using namespace wspp ;

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
        logger_(logger_dir, true), root_(root_dir), Server("127.0.0.1", port, logger_) {
    }

    void handle(const Request &req, Response &resp) override {

        sqlite::Connection con(root_ + "/db.sqlite") ;
        Session session(sm_, req, resp) ;

        UserController user(req, resp, con, session) ;
        // request router

        Dictionary attributes ;
        if ( req.matches("GET", "/page/{id:a}?", attributes) ) handlePage(resp, user, attributes.get("id")) ;
        else if ( req.matches("GET", "/post/{id:n}?", attributes) ) handlePost(resp,  attributes.get("id")) ;
        else if ( req.matches("GET", "/posts/{category:a}?", attributes) ) handlePosts(resp, attributes.get("category")) ;
        else if ( req.matches("POST", "/user/login/") ) user.login() ;
        else if ( req.matches("POST", "/user/logout/") ) user.logout() ;
        else if ( req.matches("GET", "/{fpath:**}", attributes) ) {
            string fpath = attributes.get("fpath") ;
            resp.encode_file(root_ + fpath);
        }
        else resp.stock_reply(Response::not_found) ;
    }

    void handlePage(Response &response, const UserController &user, const string &page_id) {
        string page_title = "hello" ;
        string nav_brand = "blog" ;
        string page_content = "djwljwdwlfjwlwjf" ;
#include "templates/page.tpp"
    }

    void handlePost(Response &response, const string &post_id) {

    }

    void handlePosts(Response &response, const string &category) {

    }


    FileSystemSessionHandler sm_ ;
    DefaultLogger logger_ ;
    string root_ ;
};


int main(int argc, char *argv[]) {

    TemplateRenderer rdr ;

    rdr.renderString(R"(
                     {{#repo}}
                       <b>{{name}}</b>
                     {{/repo}}
                     {{^repo}}
                       No repos :(
                     {{/repo}}
                     )", Variant::Object{
                          {"repo",
                                Variant::Object{{"name", 2.0}
                                }
                      }}) ;

    MyServer server( "5000", "/home/malasiot/source/ws/data/blog/", "/tmp/logger") ;
    server.run() ;
}
