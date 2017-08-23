#ifndef __WSPP_UTIL_TEMPLATE_RENDERER_HPP__
#define __WSPP_UTIL_TEMPLATE_RENDERER_HPP__

#include <string>
#include <iostream>

#include <wspp/util/variant.hpp>

namespace wspp {

class TemplateRenderer {
public:

    // render a template file with optional caching based on file names and file modification time stamps
    std::string renderFile(const std::string &path, const Variant::Object &context, bool cache = true) ;

    std::string renderString(const std::string &str, const Variant::Object &context) ;

private:

    std::string render(const std::string &str, const Variant::Object &context) ;
} ;

}

#endif
