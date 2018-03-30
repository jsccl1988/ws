#ifndef __WSPP_TWIG_EVAL_CONTEXT_HPP__
#define __WSPP_TWIG_EVAL_CONTEXT_HPP__

#include <memory>

#include <wspp/util/variant.hpp>

namespace wspp { namespace twig {


namespace detail {
class NamedBlockNode ;
typedef std::shared_ptr<NamedBlockNode> NamedBlockNodePtr ;

class DocumentNode ;
typedef std::shared_ptr<DocumentNode> DocumentNodePtr ;
}

class TemplateRenderer ;

struct TemplateEvalContext {

    TemplateEvalContext(TemplateRenderer &rdr, const wspp::util::Variant::Object &data):
        rdr_(rdr), data_(data) {}

    TemplateEvalContext() = delete ;

    wspp::util::Variant::Object &data() {
        return data_ ;
    }

    void addBlock(detail::NamedBlockNodePtr node) ;

    wspp::util::Variant::Object data_ ;
    std::map<std::string, detail::NamedBlockNodePtr> blocks_ ;
    TemplateRenderer &rdr_ ;
};

} // namespace twig
} // namespace wspp

#endif
