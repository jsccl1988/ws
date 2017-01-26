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

        boost::smatch m ;

        // test if the request path is what expected

        if ( req.method_ != "GET" ||
             !boost::regex_match(req.path_, m, boost::regex(R"(/hello/([a-zA-Z]+))"))  )
            return false ;

        Session session(sm, req, resp) ;

        string user = m.str(1) ;
        ostringstream strm ;

        strm << "hello " << user ;
        resp.content_ = strm.str() ;

        resp.headers_.add("Content-Length", to_string(resp.content_.size())) ;
        resp.headers_.add("Content-Type", "text/html" ) ;

        resp.status_ = Response::ok ;

        return true ;
    }
};

WSX_DECLARE_PLUGIN(MyHandler)
