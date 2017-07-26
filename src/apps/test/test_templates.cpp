#include <string>
#include <sstream>
#include <iostream>

#include <wspp/server/request_handler.hpp>
#include <wspp/server/response.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/session_manager.hpp>
#include <wspp/server/session.hpp>
#include <wspp/server/server.hpp>

#include <wspp/util/logger.hpp>
#include <wspp/util/database.hpp>

#include <iostream>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>

#include <wspp/util/variant.hpp>
using namespace std ;
using namespace wspp ;

class DefaultLogger: public Logger
{
public:
    DefaultLogger(const std::string &log_file, bool debug) {
        if ( debug ) addAppender(std::make_shared<LogStreamAppender>(Trace, make_shared<LogPatternFormatter>("%In function %c, %F:%l: %m"), std::cerr)) ;
        addAppender(std::make_shared<LogFileAppender>(Info, make_shared<LogPatternFormatter>("%V: %d %r: %m"), log_file)) ;
    }
};


class MyServer: public Server {

public:
    MyServer(const std::string &port, const std::string &logger_dir): logger_(logger_dir, true), Server("127.0.0.1", port, sm_, logger_) {
    }

    void deleteUser(const Request& req, Response& resp, Session &session, int user) {

        // all rendering is done in there
        render(resp, user) ;

        resp.setContentLength() ;
        resp.setContentType("text/html") ;

        session.data_["user_name"] = user ;

        resp.setStatus(Response::ok) ;
    }

    void showUser(Response& resp, int user) {
        try {
            sqlite::Connection con("/home/malasiot/tmp/db.sqlite") ;

            sqlite::Query stmt(con, "SELECT name, password FROM users WHERE id = ? LIMIT 1", user) ;

            sqlite::QueryResult res = stmt.exec() ;
/*
            for ( auto it: res ) {
                cout << it["name"].as<std::string>() << endl ;
            }
*/
            if ( res ) {
                Variant v(Variant::Object{{"user", res.get<string>("name")}, {"password", res.get<string>("password")}}) ;

                resp.writeJSON(v.toJSON()) ;
            }
            else
                resp.stock_reply(Response::bad_request) ;

        }
        catch ( sqlite::Exception &e ) {
            LOG_X_STREAM(logger_, Debug, e.what()) ;
            throw e ;
        }


    }

    void addUser(Response& resp, const string &name, const string &password) {
        try {
            sqlite::Connection con("/home/malasiot/tmp/db.sqlite") ;
            sqlite::Statement stmt(con, "INSERT INTO users (name, password) VALUES (?, ?)", name, password) ;
            stmt.exec() ;
        }
        catch ( sqlite::Exception &e ) {
            LOG_X_STREAM(logger_, Debug, e.what()) ;
            throw e ;
        }

    }

    virtual void handle(ConnectionContext &con) {

        // request router

        const Request &req = con.request() ;
        Response &resp = con.response() ;

        Dictionary attributes ;
        if ( req.matches("GET|POST", "/delete/{id:n}", attributes) ) deleteUser(req, resp, con.session(), attributes.value<int>("id", -1)) ;
        else if ( req.matches("GET", "/show/{id:n}?", attributes) ) showUser(resp,  attributes.value<int>("id", -1)) ;
        else if ( req.matches("GET", "/add/{name:a}/{password:a}", attributes) ) addUser(resp, attributes.get("name"), attributes.get("password")) ;
        else if ( req.matches("GET", "/data/{fpath:**}", attributes) ) {
            string fpath = attributes.get("fpath") ;
            resp.encode_file("/home/malasiot/Downloads/" + fpath);
        }
        else resp.stock_reply(Response::not_found) ;
    }

    void render(Response &response, int id) {
        string key = to_string(id) ;
#include "test_app/templates/test.tpp"
    }

    MemSessionManager sm_ ;
    DefaultLogger logger_ ;
};




int main(int argc, char *argv[]) {

    MyServer server( "5000", "/tmp/logger") ;
    server.run() ;
}
