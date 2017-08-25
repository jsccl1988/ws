#ifndef __WSPP_UTIL_TEMPLATE_RENDERER_HPP__
#define __WSPP_UTIL_TEMPLATE_RENDERER_HPP__

#include <string>
#include <iostream>

#include <boost/variant.hpp>
#include <wspp/util/variant.hpp>

namespace wspp {

typedef std::map<std::string, std::string> Partials ;

// Mustache template engine implementation

class TemplateRenderer {
public:

    // render a template file or template string using given context and list of partials
    // if the resource is a file the string has to be prefixed by '@' which is relative to the root folder
    // partials is a map of key value pairs i.e. mapping partial keys in the template to actual resources (template string or files)
    // for convenience if a partial key starts with @ then corresponding resource will be @key
    // A global cache is used to store compiled templates if caching is enabled

    std::string render(const std::string &pathOrString, const Variant &context, const Partials &partials = Partials()) ;

    // enable caching of top level templates (disable for debugging the templates)
    void setCaching(bool enable = true) {
        caching_ = enable ;
    }

    void setRootFolder(const std::string &folder) {
        root_folder_ = folder ;
    }

private:

    std::string root_folder_ ;
    bool caching_ = false ;
} ;

}

#endif
