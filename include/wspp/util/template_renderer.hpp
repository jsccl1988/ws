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

    // Render a template using given context and list of partials.
    // The loader is used to map the given key to a file or other resource.
    // A global cache is used to store compiled templates if caching is enabled.
    //
    // A simple form of dynamic partials is implemented by testing of the key is a variable in the current context. The corresponding value is then used as partial key.
    // Partials can take arguments in the form of "{{> <name> <arg1>="<val1>" <arg2>="<val2>" ... }}". The arguments are pushed in the context stack before evaluting the partial.
    // The implementation also supports an extension of mustache namely extensions and blocks similar to
    // https://github.com/mustache/spec/issues/38
    // Also the substitution pattern {{.}} is used to render the current array element in an array section (in this case the data object should point to an array of literals)
    // Lambdas are not supported.

    std::string render(const std::string &key, const Variant &context) ;

private:

    boost::shared_ptr<TemplateLoader> loader_ ;
    bool caching_ ;
} ;

}

#endif
