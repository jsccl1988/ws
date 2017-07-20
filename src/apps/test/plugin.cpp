#include <wspp/server/request_handler.hpp>
#include <wspp/server/response.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/session_manager.hpp>
#include <wspp/server/session.hpp>

#include <iostream>
#include <boost/regex.hpp>


using namespace std ;
using namespace wspp ;

class MyHandler: public RequestHandler {

public:
    MyHandler(): RequestHandler() {}

    virtual bool handle(Request& req, Response& resp, SessionManager &sm) {

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
