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
#include <wspp/util/crypto.hpp>
#include <wspp/util/variant.hpp>
#include <wspp/util/filesystem.hpp>

#include <wspp/twig/renderer.hpp>
#include <wspp/twig/functions.hpp>
#include <wspp/views/menu.hpp>

#include <boost/make_shared.hpp>

#include <iostream>

#include "login.hpp"

#include "page_controller.hpp"
#include "users_controller.hpp"
#include "route_controller.hpp"
#include "attachment_controller.hpp"
#include "wpts_controller.hpp"

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>

#include <wspp/server/route.hpp>
#include <wspp/server/filters/request_logger.hpp>
#include <wspp/server/filters/static_file_handler.hpp>
#include <wspp/server/filters/gzip_filter.hpp>

#include <spatialite.h>
#include <wspp/util/i18n.hpp>

#include <wspp/util/variant.hpp>
#include "gpx_parser.hpp"

#include <wspp/database/connection.hpp>

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;
using namespace wspp::twig ;
using namespace wspp::server ;
using namespace wspp::db ;
namespace fs = boost::filesystem ;

class SpatialLiteSingleton
{
public:

    static SpatialLiteSingleton& instance() {
        return instance_ ;
    }
    static SpatialLiteSingleton instance_;
private:

    SpatialLiteSingleton () {
        spatialite_init(false);
    }

    ~SpatialLiteSingleton () {
        spatialite_cleanup();
    }

    SpatialLiteSingleton( SpatialLiteSingleton const & ) = delete;

    void operator = ( SpatialLiteSingleton const & ) = delete ;
};

SpatialLiteSingleton SpatialLiteSingleton::instance_ ;

class DefaultLogger: public Logger
{
public:
    DefaultLogger(const std::string &log_file, bool debug) {
        if ( debug ) addAppender(std::make_shared<LogStreamAppender>(Trace, make_shared<LogPatternFormatter>("%In function %c, %F:%l: %m"), std::cerr)) ;
        addAppender(std::make_shared<LogFileAppender>(Info, make_shared<LogPatternFormatter>("%V [%d{%c}]: %m"), log_file)) ;
    }
};

class RoutesApp: public RequestHandler {
public:

    RoutesApp(const std::string &root_dir, SessionHandler &session_handler):
        session_handler_(session_handler),
        root_(root_dir),
        engine_(std::shared_ptr<TemplateLoader>(new FileSystemTemplateLoader({{root_ + "/templates/"}, {root_ + "/templates/bootstrap-partials/"}})))
    {
        i18n::instance().setLanguage("el") ;

        FunctionFactory::instance().registerFunction("_", [&](const Variant &args, TemplateEvalContext &ctx) -> Variant {
            Variant::Array unpacked ;
            unpack_args(args, { { "str", true } }, unpacked) ;
            return engine_.renderString(i18n::instance().trans(unpacked[0].toString()), ctx.data()) ;
        }) ;

        FunctionFactory::instance().registerFunction("make_two_columns", [&](const Variant &args, TemplateEvalContext &ctx) -> Variant {

            Variant::Array params ;
            unpack_args(args, { { "src", true }, {"params", true} }, params) ;
            Variant v = params.at(1) ; // parameters is the array to iterate
            string src = params[0].toString() ;

            size_t len = v.length() ;
            size_t len1 = floor(len/2.0) ;
            size_t k = 0 ;

            string res ;
            for(size_t i = 0 ; i < len1 ; i++ ) {
                Variant p1 = v.at(k++) ;
                res += "<tr>" ;

                Variant::Object cdata ;

                for ( auto it=p1.begin() ; it != p1.end() ; ++it )
                    cdata[it.key()] = it.value() ;

                res += engine_.renderString(src, cdata) ;

                Variant p2 = v.at(k++) ;

                for ( auto it=p1.begin() ; it != p1.end() ; ++it )
                    cdata[it.key()] = it.value() ;

                res += engine_.renderString(src, cdata) ;

                res += "</tr>" ;
            }
            if ( k < len ) {
                Variant p = v.at(k) ;
                res += "<tr>" ;
                Variant::Object cdata ;

                for ( auto it=p.begin() ; it != p.end() ; ++it )
                    cdata[it.key()] = it.value() ;

                res += engine_.renderString(src, cdata) ;

                res += "<td></td></tr>" ;
            }
            return res ;


        }) ;
    }

    void handle(const Request &req, Response &resp) override {

        Connection con("sqlite:db=" + root_ + "/routes.sqlite") ; // establish connection with database

        Session session(session_handler_, req, resp) ; // start a new session

        DefaultAuthorizationModel auth(Variant::fromJSONFile(root_ + "templates/acm.json")) ;
        User user(req, resp, session, con, auth) ; // setup authentication

        PageView page(user, Variant::fromJSONFile(root_ + "templates/menu.json")) ; // global page data

        // request router

        if ( RouteController(req, resp, con, user, engine_, page).dispatch() ) return ;
        if ( req.matches("GET", "/map/") ) {
            Variant::Object ctx{
                         { "page", page.data("map", _("Routes Map")) }
            } ;

            resp.write(engine_.render("map", ctx)) ;
            return ;

        }
        if ( AttachmentController(req, resp, con, user, engine_, root_ + "/data/uploads/").dispatch() ) return ;
        if ( WaypointController(req, resp, con, user, engine_).dispatch() ) return ;
        if ( PageController(req, resp, con, user, engine_, page).dispatch() ) return ;
        if ( UsersController(req, resp, con, user, engine_, page).dispatch() ) return ;
        if ( LoginController(user, req, resp, engine_).dispatch() ) return ;

        // not matched : try static file
        fs::path p(root_ + req.path_)  ;
        if ( fs::exists(p) && fs::is_regular_file(p) ) {
            resp.encodeFile(p.string());
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

    wspp::db::Connection con("uri:file:///home/malasiot/source/ws/data/routes/pg.dsn") ;

    // example of seting up translation with boost::locale
    //
    // xgettext -c++ --keyword=__ --output messages.pot main.cpp ...
    // sed --in-place messages.pot --expression='s/CHARSET/UTF-8/'
    // msginit --input=messages.pot --no-translator --locale=es_ES.UTF-8 --output es_ES/LC_MESSAGES/messages.po
    // -- translate the file: es_ES/LC_MESSAGES/messages.po
    // msgfmt --output-file=es_ES/LC_MESSAGES/messages.mo es_ES/LC_MESSAGES/messages.po

    i18n::instance().addDomain("messages") ;
    i18n::instance().addPath(".") ;

  //  Server server("vision.iti.gr", "5000") ;
    Server server("127.0.0.1", "5000") ;

    FileSystemSessionHandler sh ;
    DefaultLogger logger("/tmp/logger", true) ;

    const string root = "/home/malasiot/source/ws/data/routes/" ;
    RoutesApp *service = new RoutesApp(root, sh) ;

    server.setHandler(service) ;

    server.addFilter(new RequestLoggerFilter(logger)) ;
    server.addFilter(new GZipFilter()) ;

    server.run() ;
}
