#include <wspp/views/menu.hpp>

using namespace std;

namespace wspp {
namespace views {
Menu &Menu::addItem(const std::string &id, const std::string &title, const std::string &url){
    assert( is_submenu_ );

    Menu::Ptr item(new Menu);
    item->is_submenu_ = false;
    item->id_ = id;
    item->href_ = url;
    item->label_ = title;
    item->parent_ = this;
    children_.push_back(item);

    return *this;
}

Menu &Menu::addSubMenu(const std::string &id, const std::string &title){
    assert( is_submenu_ );

    Menu::Ptr item(new Menu);
    item->is_submenu_ = true;
    item->id_ = id;
    item->label_ = title;
    children_.push_back(item);
    item->parent_ = this;

    return *item;
}

Variant Menu::recurse(const string &current_id) const {
    if ( is_submenu_ ) {
        Variant::Object content{{"id", id_}, {"label", label_}};
        Variant::Array children;
        for( const auto &c: children_ ) {
            children.push_back(c->recurse(current_id));
        }
        content.insert({"items", children});
        return content;
    } else {
        Variant::Object content{{"id", id_}, {"label", label_}, {"href", href_}};
        if ( current_id == id_ )
            content.insert({"active", true});
        return content;
    }
}

Variant Menu::data(const string &current_id) const{
    Variant::Array items;

    for( const auto &c: children_ ) {
        items.emplace_back(c->recurse(current_id));
    }

    return Variant::Object{{"items", items}};
}
}
}