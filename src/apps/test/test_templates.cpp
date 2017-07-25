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

#include <iostream>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>

using namespace std ;
using namespace wspp ;

class MyHandler: public RequestHandler {

public:
    MyHandler(): RequestHandler() {}

    void deleteUser(const Request& req, Response& resp, Session &session, int user) {

        // all rendering is done in there
        render(resp, user) ;

        resp.setContentLength() ;
        resp.setContentType("text/html") ;

        session.data_["user_name"] = user ;

        resp.setStatus(Response::ok) ;
    }

    void showUser(Response& resp, const string &what, int user) {
        resp.write("<html>hello</html>") ;
    }


    virtual void handle(const Request& req, Response& resp, Session &session) {

        // request router

        Dictionary attributes ;
        if ( req.matches("GET|POST", "/delete/{id:n}", attributes) ) deleteUser(req, resp, session, attributes.value<int>("id", -1)) ;
        else if ( req.matches("GET", "/show/{what:*}/{id:n}?", attributes) ) showUser(resp,  attributes.get("what"), attributes.value<int>("id", -1)) ;
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
};


class DefaultLogger: public Logger
{
public:
    DefaultLogger(const std::string &log_file, bool debug) {
        if ( debug ) addAppender(std::make_shared<LogStreamAppender>(Trace, make_shared<LogPatternFormatter>("%In function %c, %F:%l: %m"), std::cerr)) ;
        if ( boost::filesystem::exists(log_file) )
            addAppender(std::make_shared<LogFileAppender>(Info, make_shared<LogPatternFormatter>("%V: %d %r: %m"), log_file)) ;
    }
};

std::unique_ptr<DefaultLogger> g_server_logger ;

Logger &get_current_logger() {
    return *g_server_logger ;
}

template<class T>
string toJSON(const T &val) {
    stringstream strm ;
    strm << val ;
    return strm.str() ;
}

template<class T>
void toJSONObject(const std::map<string, T> &obj) {
    stringstream strm ;
    strm << "{" ;
    for( auto a: obj ) {
        strm << a.first << ':' << toJSON(a.second) << "," ;
    }

    strm << "}" ;

    return strm.str() ;
}

int main(int argc, char *argv[]) {

    g_server_logger.reset(new DefaultLogger("/tmp/server-log", true)) ;

    MemSessionManager sm ;

    Server server(boost::make_shared<MyHandler>(), "127.0.0.1", "5000", sm, 10) ;


    server.run() ;
}
