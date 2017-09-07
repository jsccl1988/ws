#include "page_view.hpp"

using namespace std ;
using namespace wspp::server ;

PageView::PageView(const Authentication &user): auth_(user) {
    menu_.addItem("home", "Home", "/") ;
    menu_.addItem("maps", "Maps", "/maps") ;
    menu_.addItem("segments", "Segments", "/segments") ;
    menu_.addSubMenu("about", "About")
            .addItem("faq", "FAQ", "/faq")
            .addItem("contact", "Contact", "/contact") ;

}

Variant PageView::data(const std::string &page_id, const std::string &title) const {

    Variant::Object nav{{"menu", menu_.data(page_id) }} ;
    if ( auth_.check() ) nav.insert({"username", auth_.userName()}) ;

    Variant page(
                Variant::Object{
                    { "nav", nav },
                    { "title", title }
                }
            ) ;

    return page ;
}
