#ifndef __WSPP_TEMPLATE_RENDERER_HPP__
#define __WSPP_TEMPLATE_RENDERER_HPP__

#include <memory>

#include "template_loader.hpp"
#include "template_exceptions.hpp"

#include <wspp/util/variant.hpp>
#include <boost/thread/mutex.hpp>

namespace ast {
    class DocumentNode ;
    class ExtensionBlockNode ;
    class ImportBlockNode ;
    typedef std::shared_ptr<DocumentNode> DocumentNodePtr ;
}


class TemplateRenderer {
public:
    TemplateRenderer(std::shared_ptr<TemplateLoader> loader): loader_(loader) {}

    std::string render(const std::string &resource, const wspp::util::Variant::Object &ctx) ;
    std::string renderString(const std::string &str, const wspp::util::Variant::Object &ctx) ;

    bool setDebug(bool debug = true) {
        debug_ = debug ;
    }

    bool setCaching(bool cache = true) {
        caching_ = cache ;
    }

protected:

    class Cache {
    public:

        using Entry = ast::DocumentNodePtr ;

        void add(const std::string &key, const Entry &val) {
            boost::mutex::scoped_lock lock(guard_);
            compiled_.insert({key, val}) ;
        }

        Entry fetch(const std::string &key) {
            boost::mutex::scoped_lock lock(guard_);
            auto it = compiled_.find(key) ;
            if ( it != compiled_.end() ) return it->second ;
            else return nullptr ;
        }

    private:
        std::map<std::string, Entry> compiled_ ;
        boost::mutex guard_ ;

    };

    friend class ast::ExtensionBlockNode ;
    friend class ast::ImportBlockNode ;

    ast::DocumentNodePtr compile(const std::string &resource) ;
    ast::DocumentNodePtr compileString(const std::string &resource) ;

    bool debug_ = false, caching_ = true ;
    std::shared_ptr<TemplateLoader> loader_ ;
} ;




#endif
