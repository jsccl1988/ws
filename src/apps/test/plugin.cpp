#include "server/request_handler.hpp"

#include <iostream>

using namespace std ;

class MyHandler: public http::RequestHandler {

public:
    MyHandler(): http::RequestHandler() {}

    virtual void handle_request(const http::Request& req, http::Response& rep) {

        std::cout << "ok here" << endl ;
    }

    virtual bool matches(const std::string &req_path) {
        return true ;
    }
};

extern "C"
{
    http::RequestHandler *rh_create() {
        return new MyHandler() ;
    }
}
