#include "plugin_handler_factory.hpp"
#include <wspp/util/logger.hpp>

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

#include <dlfcn.h>

extern "C" {
typedef wspp::RequestHandler *(*plugin_handler_factory_t)()  ;
}

using namespace std ;
namespace fs = boost::filesystem ;

namespace wspp {

PluginHandlerFactory::PluginHandlerFactory(const std::string &user_plugin_path): RequestHandler() {

    string plugin_paths ;
    const char *env_path = getenv("WSX_PLUGINS_PATH") ;
    if ( env_path ) {
        plugin_paths += env_path ;
        plugin_paths += ';' ;
    }
    plugin_paths += user_plugin_path ;

    vector<string> folders ;
    boost::algorithm::split(folders, plugin_paths, boost::is_any_of(";:"), boost::token_compress_on);

    for( auto &folder: folders) {

        if ( fs::is_directory(folder)) {
            for(auto& entry : boost::make_iterator_range(fs::directory_iterator(folder), {})) {
                if ( fs::is_regular_file(entry) ) {

                    if ( void* handle = dlopen(entry.path().native().c_str(), RTLD_LAZY) ) { //RLTD_LAZY

                        if ( plugin_handler_factory_t hfactory = (plugin_handler_factory_t)dlsym(handle, "wsx_rh_create") ) {
                            LOG_INFO_STREAM("Loading plugin: " << entry) ;
                            handlers_.push_back(std::unique_ptr<wspp::RequestHandler>(hfactory())) ;
                        }
                    }
                }
            }
        }
    }
}

bool PluginHandlerFactory::handle(const Request &req, Response &resp, SessionManager &sm) {

    for( auto &entry: handlers_) {
        bool res = entry->handle(req, resp, sm) ;
        if ( res ) return true ;
    }
    return false ;
}


}
