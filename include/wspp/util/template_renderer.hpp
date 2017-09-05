#ifndef __WSPP_UTIL_TEMPLATE_RENDERER_HPP__
#define __WSPP_UTIL_TEMPLATE_RENDERER_HPP__

#include <string>
#include <iostream>

#include <boost/variant.hpp>
#include <wspp/util/variant.hpp>

namespace wspp {

// abstract template loader
class TemplateLoader {
public:
    // override to return a template string from a key
    virtual std::string load(const std::string &src) =0 ;
    // this is used to implement recursive loading e.g. of partials or inheritance where keys may be relative to the parent
};

// loads templates from file system relative to root folders.

class FileSystemTemplateLoader: public TemplateLoader {

public:
    FileSystemTemplateLoader(const std::initializer_list<std::string> &root_folders, const std::string &suffix = ".mst") ;

    virtual std::string load(const std::string &src) override ;
private:
    std::vector<std::string> root_folders_ ;
    std::string suffix_ ;
};

// Mustache template engine implementation

class TemplateRenderer {
public:

    TemplateRenderer( const boost::shared_ptr<TemplateLoader> &loader, bool caching = false ): loader_(loader), caching_(caching) {}

    // Render a template file or template string using given context and list of partials.
    // If the resource is a file the string has to be prefixed by '@' which is relative to the root folder.
    // "partials" is a map of key value pairs i.e. mapping partial keys in the template to actual resources (template string or files)
    // For convenience if a partial key starts with @ then corresponding resource will be @key
    // A global cache is used to store compiled templates if caching is enabled.
    // A simple form of dynamic partials is implemented by allowing partial keys to contain substitution patterns e.g. {{> @forms/%field_type%_field.mst }}
    // The pattern params are looked up in the context stack and replaced with corresponding values before rendering the partials.
    // Also the substitution pattern {{.}} is used to render the current array element in an array section (in this case the data object should point to an array of literals)
    // Lambdas are not supported.

    std::string render(const std::string &key, const Variant &context) ;

private:

    boost::shared_ptr<TemplateLoader> loader_ ;
    bool caching_ ;
} ;

}

#endif
