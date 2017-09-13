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

#include <wspp/controllers/login.hpp>

#include "page_controller.hpp"
#include "users_controller.hpp"

#include <boost/locale.hpp>
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
        engine_.registerHelper("i18n", [&](const std::string &src, ContextStack &ctx) -> string {
            return engine_.renderString(boost::locale::translate(src), ctx) ;
        }) ;
    }

    void log(const Request &req, Response &resp) {

        LOG_X_STREAM(logger_, Info, "Response to " <<
                     req.SERVER_.get("REMOTE_ADDR", "127.0.0.1")
                       << ": \"" << req.method_ << " " << req.path_
                       << ((req.query_.empty()) ? "" : "?" + req.query_) << " "
                       << req.protocol_ << "\" "
                       << resp.status_ << " " << resp.headers_.value<int>("Content-Length", 0)
                     ) ;

    }

    void handle(const Request &req, Response &resp) override {

        sqlite::Connection con(root_ + "/db.sqlite") ; // establish connection with database
        Session session(sm_, req, resp) ; // start a new session

        DefaultAuthorizationModel auth(Variant::fromJSONFile(root_ + "templates/acm.json")) ;
        User user(req, resp, session, con, auth) ; // setup authentication

        PageView page(user, Variant::fromJSONFile(root_ + "templates/menu.json")) ; // global page data

        // request router

        if ( PageController(req, resp, con, user, engine_, page).dispatch() ) return ;
        else if ( UsersController(req, resp, con, user, engine_, page).dispatch() ) return ;
        else if ( LoginController(user, req, resp, engine_).dispatch() ) return ;
        else if ( req.method_ == "GET" ) {
            resp.encode_file(root_ + req.path_);
        }
        else resp.stock_reply(Response::not_found) ;

        log(req, resp) ;
    }


    FileSystemSessionHandler sm_ ;
    DefaultLogger logger_ ;
    string root_ ;
    TemplateRenderer engine_ ;

};

#define __(S) boost::locale::translate(S)

int main(int argc, char *argv[]) {

    // example of seting up translation with boost::locale
    //
    // xgettext -c++ --keyword=__ --output messages.pot main.cpp ...
    // sed --in-place messages.pot --expression='s/CHARSET/UTF-8/'
    // msginit --input=messages.pot --no-translator --locale=es_ES.UTF-8 --output es_ES/LC_MESSAGES/messages.po
    // -- translate the file: es_ES/LC_MESSAGES/messages.po
    // msgfmt --output-file=es_ES/LC_MESSAGES/messages.mo es_ES/LC_MESSAGES/messages.po

    boost::locale::generator gen;
    gen.add_messages_domain("messages");
    gen.add_messages_path(".");

    using namespace boost::locale ;
    auto loc = gen.generate("es_ES.UTF-8") ;
    locale::global(loc) ;

    std::cout << __("hello").str() << endl ;


    MyServer server( "5000", "/home/malasiot/source/ws/data/blog/", "/tmp/logger") ;
    server.run() ;
}
