#ifndef __WSPP_TEMPLATE_RENDERER_HPP__
#define __WSPP_TEMPLATE_RENDERER_HPP__

#include <string>
#include <iostream>

#include <boost/variant.hpp>
#include <wspp/util/variant.hpp>

namespace wspp {
namespace web {
using util::Variant;
// abstract template loader
class TemplateLoader {
public:
    // override to return a template string from a key
    virtual std::string load(const std::string &src) =0;
    // this is used to implement recursive loading e.g. of partials or inheritance where keys may be relative to the parent
};

// loads templates from file system relative to root folders.
class FileSystemTemplateLoader: public TemplateLoader {
public:
    FileSystemTemplateLoader(const std::initializer_list<std::string> &root_folders, const std::string &suffix = ".mst");

    virtual std::string load(const std::string &src) override;

private:
    std::vector<std::string> root_folders_;
    std::string suffix_;
};

// This is the stack of variables passed to template renderer substitution mechanism
struct ContextStack {
    const Variant &top() const { return *stack_.back(); }
    void push(const Variant &ctx) {
        stack_.push_back(&ctx);
    }

    void pop() { stack_.pop_back(); }
    // search the stack to find a variable with matching key
    const Variant &find(const std::string &item) {
        for ( auto it = stack_.rbegin(); it != stack_.rend(); ++it ) {
            const Variant &v = (*it)->at(item);
            if ( !v.isNull() ) return v;
        }

        return Variant::null();
    }

    std::deque<const Variant *> stack_;
};

// Mustache template engine implementation
class TemplateRenderer {
public:
    TemplateRenderer( const std::shared_ptr<TemplateLoader> &loader, bool caching = false ): loader_(loader), caching_(caching) {
        registerDefaultHelpers();
    }

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
    std::string render(const std::string &key, const Variant &context);

    // Render a mustache template into a string given the context
    // Usefull for helper functions
    std::string renderString(const std::string &content, ContextStack &context);

    // Block helpers registration
    typedef std::function<std::string(const std::string &src, ContextStack &stack, Variant::Array params)> BlockHelper;
    void registerBlockHelper(const std::string &name,  BlockHelper helper);

    // Value helpers registration (the helper returns also boolean flag indicating whether the output needs escaping)
    typedef std::function< std::pair<bool, std::string>(ContextStack &stack, Variant::Array params)> ValueHelper;
    void registerValueHelper(const std::string &name,  ValueHelper helper);

private:
    void registerDefaultHelpers();

    std::map<std::string, BlockHelper> block_helpers_;
    std::map<std::string, ValueHelper> value_helpers_;

    std::shared_ptr<TemplateLoader> loader_;
    bool caching_;
};
} // namespace web
} // namespace wspp
#endif
