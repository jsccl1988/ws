#ifndef __WSPP_TEMPLATE_RENDERER_HPP__
#define __WSPP_TEMPLATE_RENDERER_HPP__

#include <string>
#include <iostream>

#include <boost/variant.hpp>
#include <wspp/util/variant.hpp>

namespace wspp { namespace web {

using util::Variant ;

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
class ContextStack ;

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
    // Lambdas are not supported but helpers can be registered. If a section tag corresponds to a registered helper then the section text is passed to the helper callback
    // function along with the current context stack. The helper should then call renderString, modify it as needed and return it to the caller.

    std::string render(const std::string &key, const Variant &context) ;

    std::string renderString(const std::string &content, ContextStack &context) ;

    typedef std::function<std::string(const std::string &src, ContextStack &stack)> Helper ;
    void registerHelper(const std::string &name,  Helper helper) ;

private:

    std::map<std::string, Helper> helpers_ ;

    boost::shared_ptr<TemplateLoader> loader_ ;
    bool caching_ ;
} ;

} // namespace web

} // namespace wspp
#endif
