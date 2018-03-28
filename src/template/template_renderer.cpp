#include "template_renderer.hpp"
#include "parser.hpp"

using namespace std ;
using namespace ast ;

ast::DocumentNodePtr TemplateRenderer::compile(const std::string &resource)
{
    assert(loader_) ;
    string src = loader_->load(resource) ;

    istringstream strm(src) ;

    TemplateParser parser(strm) ;

    ast::DocumentNodePtr root(new ast::DocumentNode()) ;

    if ( !parser.parse(root) ) {
        if ( debug_ ) {
            std::shared_ptr<RawTextNode> errorText(new RawTextNode(parser.getErrorString())) ;
            root->addChild(errorText) ;
        }
    }

    return root ;
}
