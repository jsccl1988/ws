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
#include "route_controller.hpp"

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>

#include <wspp/server/route.hpp>
#include <wspp/server/filters/request_logger.hpp>
#include <wspp/server/filters/static_file_handler.hpp>


using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;
using namespace wspp::server ;
namespace fs = boost::filesystem ;

class DefaultLogger: public Logger
{
public:
    DefaultLogger(const std::string &log_file, bool debug) {
        if ( debug ) addAppender(std::make_shared<LogStreamAppender>(Trace, make_shared<LogPatternFormatter>("%In function %c, %F:%l: %m"), std::cerr)) ;
        addAppender(std::make_shared<LogFileAppender>(Info, make_shared<LogPatternFormatter>("%V [%d{%c}]: %m"), log_file)) ;
    }
};

class BlogService: public RequestHandler {
public:

    BlogService(const std::string &root_dir, SessionHandler &session_handler):
        session_handler_(session_handler),
        root_(root_dir),
        engine_(boost::shared_ptr<TemplateLoader>(new FileSystemTemplateLoader({{root_ + "/templates/"}, {root_ + "/templates/bootstrap-partials/"}})))
    {
        engine_.registerHelper("i18n", [&](const std::string &src, ContextStack &ctx) -> string {
            return engine_.renderString(boost::locale::translate(src), ctx) ;
        }) ;
    }

    void handle(const Request &req, Response &resp) override {

        sqlite::Connection con(root_ + "/db.sqlite") ; // establish connection with database
        Session session(session_handler_, req, resp) ; // start a new session

        DefaultAuthorizationModel auth(Variant::fromJSONFile(root_ + "templates/acm.json")) ;
        User user(req, resp, session, con, auth) ; // setup authentication

        PageView page(user, Variant::fromJSONFile(root_ + "templates/menu.json")) ; // global page data

        // request router

        if ( RouteController(req, resp, con, user, engine_, page).dispatch() ) return ;
        if ( PageController(req, resp, con, user, engine_, page).dispatch() ) return ;
        if ( UsersController(req, resp, con, user, engine_, page).dispatch() ) return ;
        if ( LoginController(user, req, resp, engine_).dispatch() ) return ;

        fs::path p(root_ + req.path_)  ;
        if ( fs::exists(p) && fs::is_regular_file(p) ) {
            resp.encode_file(p.string());
            return ;
        }

        throw HttpResponseException(Response::not_found) ;

    }


private:

    SessionHandler &session_handler_ ;
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

    Server server("127.0.0.1", "5000") ;

    FileSystemSessionHandler sh ;
    DefaultLogger logger("/tmp/logger", true) ;

    const string root = "/home/malasiot/source/ws/data/routes/" ;
    BlogService *service = new BlogService(root, sh) ;

    server.setHandler(service) ;

    server.addFilter(new RequestLoggerFilter(logger)) ;

    server.run() ;
}
