#ifndef __WSPP_TEMPLATE_RENDERER_HPP__
#define __WSPP_TEMPLATE_RENDERER_HPP__

#include "template_loader.hpp"
#include "template_ast.hpp"

class TemplateRenderer {
public:
    TemplateRenderer(std::shared_ptr<TemplateLoader> loader): loader_(loader) {}

    ast::DocumentNodePtr compile(const std::string &resource) ;

    bool setDebug(bool debug = true) {
        debug_ = debug ;
    }

private:

    bool debug_ = false, cache_ = false ;
    std::shared_ptr<TemplateLoader> loader_ ;


} ;


#endif
