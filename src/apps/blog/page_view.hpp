#ifndef __PAGE_VIEW_HPP__
#define __PAGE_VIEW_HPP__

#include <wspp/views/menu.hpp>
#include <wspp/util/variant.hpp>

#include <wspp/controllers/user_controller.hpp>

using wspp::web::UserController ;
using wspp::util::Variant ;

// Helper for global page layout

class PageView {
 public:

    PageView(const UserController &user);

    Variant data(const std::string &page_id, const std::string &title) const;

    wspp::views::Menu menu_ ;
    const UserController &user_ ;
} ;






#endif
