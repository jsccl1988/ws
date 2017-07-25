#ifndef __HTTP_PLUGIN_HANDLER_FACTORY_HPP__
#define __HTTP_PLUGIN_HANDLER_FACTORY_HPP__

#include <wspp/server/request_handler.hpp>

#include <vector>

namespace wspp {

class PluginHandlerFactory: public RequestHandler {
public:
    // This handler will look in a list of directories for web app plugins
    // These include the paths provided in WSX_PLUGINS_PATH environment variable and the plugin_folders variable
    // Folders in the path string should be separated with ';' or ':'

    PluginHandlerFactory(const std::string &plugin_folders) ;

    void handle(Request &req, Response &resp, SessionManager &) override ;

private:

    std::vector<std::unique_ptr<RequestHandler>> handlers_ ; // list of loaded handlers
};


} // namespace http

#endif
