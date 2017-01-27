#include "server/request_handler.hpp"
#include "server/reply.hpp"
#include "server/request.hpp"
#include "server/session_manager.hpp"
#include "server/session.hpp"

#include <iostream>
#include <boost/regex.hpp>

using namespace std ;
using namespace http ;

class MyHandler: public http::RequestHandler {

public:
    MyHandler(): http::RequestHandler() {}

    virtual bool handle(const http::Request& req, http::Response& resp, http::SessionManager &sm) {

        // test if the request path is what expected

        string user ;
        if ( !req.matches("GET", R"(/hello/([a-zA-Z]+))", user) ) return false ;

        Session session ;
        sm.open(req, session) ;

        resp.content_ = "hello " + user ;

        resp.headers_.add("Content-Length", to_string(resp.content_.size())) ;
        resp.headers_.add("Content-Type", "text/html" ) ;

        session.data_["user_name"] = user ;

        resp.status_ = Response::ok ;

        sm.close(resp, session) ;

        return true ;
    }
};

WSX_DECLARE_PLUGIN(MyHandler)
