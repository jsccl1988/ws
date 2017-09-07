#ifndef __PAGE_VIEW_HPP__
#define __PAGE_VIEW_HPP__

#include <wspp/views/menu.hpp>
#include <wspp/util/variant.hpp>

#include <wspp/models/auth.hpp>

using wspp::web::Authentication ;
using wspp::util::Variant ;

// Helper for global page layout

class PageView {
 public:

    PageView(const Authentication &user);

    Variant data(const std::string &page_id, const std::string &title) const;

    wspp::views::Menu menu_ ;
    const Authentication &auth_ ;
} ;






#endif
