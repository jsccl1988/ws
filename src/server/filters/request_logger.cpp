#include <wspp/server/filters/request_logger.hpp>

#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/server/filter_chain.hpp>
#include <wspp/server/exceptions.hpp>

#include <wspp/util/database.hpp>

using namespace wspp::util ;

namespace wspp { namespace server {

void RequestLoggerFilter::handle(Request &req, Response &resp, FilterChain &chain) {
    try {
        chain.next(req, resp) ;
        LOG_X_STREAM(logger_, Info, "Response to " <<
                 req.SERVER_.get("REMOTE_ADDR", "127.0.0.1")
                 << ": \"" << req.method_ << " " << req.path_
                 << ((req.query_.empty()) ? "" : "?" + req.query_) << " "
                 << req.protocol_ << "\" "
                 << resp.status_ << " " << resp.headers_.value<int>("Content-Length", 0)
                 ) ;
    }
    catch ( HttpResponseException &e ) {
        LOG_X_STREAM(logger_, Error, "Response to " <<
                 req.SERVER_.get("REMOTE_ADDR", "127.0.0.1")
                 << ": \"" << req.method_ << " " << req.path_
                 << ((req.query_.empty()) ? "" : "?" + req.query_) << " "
                 << req.protocol_ << "\" "
                 << resp.status_
                 ) ;
        throw e ;
    }
    catch ( sqlite::Exception &e ) {
        LOG_X_STREAM(logger_, Debug, "SQL error: " << e.what()) ;
        throw e ;
    }
}

} // namespace server
} // namespace wspp
