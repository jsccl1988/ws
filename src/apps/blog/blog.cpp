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
#include "page_controller.hpp"
#include "menu_controller.hpp"

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
        engine_.setRootFolder(root_ + "/templates/");
    }

    void handle(const Request &req, Response &resp) override {

        sqlite::Connection con(root_ + "/db.sqlite") ;
        Session session(sm_, req, resp) ;

        UserController user(req, resp, con, session) ;

        // request router

        Dictionary attributes ;


        if ( req.matches("GET", "/page/add/", attributes) ) {
            PageController pages(req, resp, con, user, engine_) ;
            pages.create() ;
        }
        else if ( req.matches("GET", "/page/list/{pager:n}?", attributes) ) {
            PageController pages(req, resp, con, user, engine_) ;
            pages.list(attributes.value<int>("pager", 0)) ;
        }
        else if ( req.matches("GET", "/page/edit/{id:a}", attributes) ) {
            PageController pages(req, resp, con, user, engine_) ;
            pages.edit(attributes.get("id")) ;
        }
        else if ( req.matches("POST", "/page/publish") ) {
            PageController pages(req, resp, con, user, engine_) ;
            pages.publish() ;
        }
        else if ( req.matches("POST", "/page/delete") ) {
            PageController pages(req, resp, con, user, engine_) ;
            pages.remove() ;
        }
        else if ( req.matches("GET", "/page/{id:a}", attributes) ) {
            PageController pages(req, resp, con, user, engine_) ;
            pages.show(attributes.get("id")) ;
        }
        else if ( req.matches("GET", "/menu/edit/") ) {
            MenuController menus(req, resp, con, user, engine_) ;
            menus.edit() ;
        }
        else if ( req.matches("GET|POST", "/menu/create/") ) {
            MenuController menus(req, resp, con, user, engine_) ;
            menus.create() ;
        }
        else if ( req.matches("POST", "/menu/delete/") ) {
            MenuController menus(req, resp, con, user, engine_) ;
            menus.remove() ;
        }
        else if ( req.matches("GET", "/menu/list/") ) {
            MenuController menus(req, resp, con, user, engine_) ;
            menus.fetch() ;
        }

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
        // CREATE TABLE pages (id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT, content TEXT);

        sqlite::Connection con(root_ + "/db.sqlite") ;
        sqlite::Query q(con, "SELECT title, content FROM pages WHERE id=?;", page_id) ;
        sqlite::QueryResult res = q.exec() ;

        if ( res ) {

            Variant ctx( Variant::Object{
                         {"page", Variant::Object{
                                    { "id", page_id },
                                    { "title", res.get<string>("title") },
                                    { "content", res.get<string>("content") }
                                  }
                         },
                         {"nav_brand", "blog"},
                         {"logged_in", user.isLoggedIn()},
                         {"user_name", user.name()}}) ;
            response.write(engine_.render("@page.mst", ctx)) ;
        }
        else
            response.stock_reply(Response::not_found);
    }

    void handlePost(Response &response, const string &post_id) {

    }

    void handlePosts(Response &response, const string &category) {

    }


    FileSystemSessionHandler sm_ ;
    DefaultLogger logger_ ;
    string root_ ;
    TemplateRenderer engine_ ;
};


int main(int argc, char *argv[]) {

    TemplateRenderer rdr ;
/*
    Variant::Object ctx{
           {  { "id", 1 },
           {  "children", Variant::Array{
                        Variant::Object{{"id", 2}, {"href", string("h1")}, {"active", true }},
                        Variant::Object{{"id", string("item2")}, {"href", string("h2")} },
                        Variant::Object{{"id", string("item3")}, {"href", string("h3")} },
                        Variant::Object{{"name", string("item4")}, {"href", string("h4")} },
                    }
           }
        }} ;

    Variant v(Variant::Object{{"node", ctx}}) ;

    cout << v.at("node.children").at(0).at("href").toJSON() << endl ;

    cout << rdr.render(R"(
                       {{ id }}
                       {{#children}}
                           {{id}}  {{! will also output node.children[i].id }}
                       {{/children}}

                     )",
                       ctx,
                     {{"partial1", "This is my {{{name}}}"}}
                    ) << endl ;
*/
    MyServer server( "5000", "/home/malasiot/source/ws/data/blog/", "/tmp/logger") ;
    server.run() ;
}
