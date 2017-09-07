#ifndef __WSPP_VIEWS_MENU_HPP__
#define __WSPP_VIEWS_MENU_HPP__

#include <string>
#include <map>

#include <boost/scoped_ptr.hpp>
#include <wspp/util/variant.hpp>

namespace wspp { namespace views {

using namespace util ;

class Menu {
public:

    Menu(): is_submenu_(true) {}

    Menu &addItem(const std::string &id, const std::string &title, const std::string &url) ;
    Menu &addSubMenu(const std::string &id, const std::string &title) ;

    std::string label() const { return label_ ; }
    std::string link() const { return href_ ; }
    std::string id() const { return id_ ; }

    Variant data(const std::string &current) const ;

    typedef boost::shared_ptr<Menu> Ptr ;

    Menu(const Menu &) = delete ;

private:

    bool is_submenu_ ;
    Menu *parent_ = nullptr ;
    std::string id_ ;
    std::string label_ ;
    std::string href_ ;
    std::vector<Menu::Ptr> children_ ;
private:

    Variant recurse(const std::string &current_id) const;
};


}
}

#endif
