#include <wspp/server/filters/static_file_handler.hpp>

#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>
#include <wspp/server/filter_chain.hpp>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

using namespace wspp::util;
namespace wspp {
namespace server {
void StaticFileHandler::handle(Request &req, Response &resp, FilterChain &chain) {
    if ( resp.status_ != Response::ok && req.method_ == "GET" ) {
        fs::path p(root_ + req.path_) ;
        if ( fs::exists(p) )
            resp.encodeFile(p.string());
    }

    chain.next(req, resp);
}
} // namespace server
} // namespace wspp
