#ifndef __UTIL_I18N_HPP__
#define __UTIL_I18N_HPP__

#include <string>
#include <boost/locale.hpp>

using boost::locale::generator;
namespace wspp {
namespace util {
class i18n {
public:
    i18n() {}
    static i18n &instance() {
        static thread_local i18n instance_;
        return instance_;
    }

    void addDomain(const std::string &domain) { gen_.add_messages_domain(domain); }
    void addPath(const std::string &path) { gen_.add_messages_path(path); }

    void setLanguage(const std::string &lname) {
        locale_ = generate(lname);
   //     std::cout << "The language code is " << std::use_facet<boost::locale::info>(locale_).language() << std::endl;
    }

    void setDomain(const std::string &domain) {
        domain_ = domain;
    }

    void setDefaultLanguage(const std::string &lang) {
        default_language_ = lang;
    }

    std::locale generate(const std::string &loc_id) {
        return gen_.generate(loc_id);
    }

    std::string setLanguageFromRequestPath(const std::string &path);

    std::string trans(const std::string &src) {
        if ( language_.empty() )
            return boost::locale::translate(src).str(generate(default_language_), domain_);
        return boost::locale::translate(src).str(locale_, domain_);
    }
private:

    generator gen_;
    std::locale locale_;
    std::string domain_, language_, default_language_ = "en";
};
} // namespace util
} // namespace wspp
#define _(S) wspp::util::i18n::instance().trans(S)
#endif
