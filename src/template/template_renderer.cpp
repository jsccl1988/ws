#include "template_renderer.hpp"
#include "parser.hpp"


using namespace std ;
using namespace detail ;

string TemplateRenderer::render(const string &resource, const Variant::Object &ctx)
{
    TemplateEvalContext eval_ctx(*this, ctx) ;

    auto ast = compile(resource) ;
    string res ;
    ast->eval(eval_ctx, res) ;
    return res ;
}

string TemplateRenderer::renderString(const string &str, const Variant::Object &ctx)
{
    TemplateEvalContext eval_ctx(*this, ctx) ;

    auto ast = compileString(str) ;
    string res ;
    ast->eval(eval_ctx, res) ;
    return res ;
}

detail::DocumentNodePtr TemplateRenderer::compile(const std::string &resource)
{
    static Cache g_cache ;

    if ( resource.empty() ) return nullptr ;

    if ( caching_ ) {
        auto stored = g_cache.fetch(resource) ;
        if ( stored ) return stored ;
    }

    string src = loader_->load(resource);

    istringstream strm(src) ;

    TemplateParser parser(strm) ;

    detail::DocumentNodePtr root(new detail::DocumentNode(*this)) ;

    parser.parse(root, resource) ;

    if ( caching_ ) g_cache.add(resource, root) ;

    return root ;
}

detail::DocumentNodePtr TemplateRenderer::compileString(const std::string &src)
{
    istringstream strm(src) ;

    TemplateParser parser(strm) ;

    detail::DocumentNodePtr root(new detail::DocumentNode(*this)) ;

    parser.parse(root, "--string--") ;

    return root ;
}
