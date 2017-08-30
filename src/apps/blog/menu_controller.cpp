#include "menu_controller.hpp"
#include <wspp/util/forms.hpp>

using namespace std ;
using namespace wspp ;

// CREATE TABLE menus (id INTEGER PRIMARY KEY AUTOINCREMENT, name UNIQUE NOT NULL, parent INTEGER DEFAULT NULL, link TEXT DEFAULT NULL);

void MenuController::add()
{
    if ( !user_.isLoggedIn() ) {
        response_.stock_reply(Response::unauthorized) ;
        return ;
    }

    const Dictionary &params = request_.POST_ ;
/*
    MenuForm form(con_) ;

    if ( form.validate(params) ) {

        sqlite::Statement stmt(con_, "INSERT INTO menus ( name, parent, link ) VALUES ( ?, ?, ? )") ;

        stmt.bind(1, name) ;
        stmt.bind(2, parent) ;
        if ( link.empty() ) stmt.bind(3, sqlite::Nil) ;
        else stmt.bind(3, link) ;
        stmt.exec() ;

        response_.writeJSONVariant(Variant::Object{{"msg", "Menu added succesfully"}, {"id", con_.last_insert_rowid()},
                                               {"name", name }, {"link", link }} ) ;
    }
    */
}

void MenuController::remove()
{
    if ( !user_.isLoggedIn() ) {
        response_.stock_reply(Response::unauthorized) ;
        return ;
    }

    const Dictionary &params = request_.POST_ ;
    string id = params.get("id") ;

    if ( id.empty() )
        response_.stock_reply(Response::not_found) ;
    else {
        sqlite::Statement stmt(con_, "DELETE FROM menus where id=?", id) ;
        stmt.exec() ;
    }
}

void MenuController::fetch()
{
    if ( !user_.isLoggedIn() ) {
        response_.stock_reply(Response::unauthorized) ;
        return ;
    }

    Variant v = fetchList() ;

    response_.writeJSONVariant(Variant::Object{{"data", v}}) ;
}



void MenuController::edit()
{
    if ( !user_.isLoggedIn() ) {
        response_.stock_reply(Response::unauthorized) ;
        return ;
    }

    MenuForm form(con_) ;

    if ( request_.method_ == "POST" ) {

        if ( form.validate(request_.POST_) ) {

            sqlite::Statement stmt(con_, "INSERT INTO menus ( name, parent, link ) VALUES ( ?, ?, ? )") ;

            stmt.bind(1, form.getValue("name")) ;
            stmt.bind(2, form.getValue("parent")) ;
            string link = form.getValue("link") ;
            if ( link.empty() ) stmt.bind(3, sqlite::Nil) ;
            else stmt.bind(3, link) ;
            stmt.exec() ;
        }
    }
    Variant ctx( Variant::Object{
                     {"page_title", Variant::Object{
                          { "title", "Edit Menu" },
                      }
                     },
                     {"menus", Variant::Object{{"list", fetchList()}}},
                     {"menus_form", form.data()},
                     {"nav_brand", "blog"},
                     {"logged_in", user_.isLoggedIn()},
                     {"user_name", user_.name()}}) ;

    response_.write(engine_.render("@menu-edit.mst", ctx)) ;
}


Variant MenuController::fetchList() {
    sqlite::Query q(con_, "SELECT t1.*, t2.name FROM menus AS t1 LEFT JOIN menus AS t2 ON t1.parent = t2.id") ;
    sqlite::QueryResult res = q.exec() ;

    Variant::Array items ;

    while ( res ) {
        items.emplace_back(Variant::Object{{"id", res.get<string>(0)},
                                           {"name", res.get<string>(1)},
                                           {"parent_id", res.get<int>(2)},
                                           {"parent_name", res.get<string>(4)},
                                           {"link", res.get<string>(3)}
                           }) ;

        res.next() ;

    }

    return items ;
}

MenuForm::MenuForm(sqlite::Connection &con): con_(con) {



    input("name", "text").label("Name:").required().addValidator([&] (const string &val, FormField &f) {
        sqlite::Query q(con_, "SELECT count(*) FROM menus WHERE name = ?", val) ;
        sqlite::QueryResult res = q.exec() ;

        if ( res.get<int>(0) ) {
            f.addErrorMsg("A menu with this name already exists") ;
            return false ;
        }
        return true ;
    }) ;

    auto fetcher = [&] () {
        sqlite::Query q(con_, "SELECT id, name FROM menus;") ;
        sqlite::QueryResult res = q.exec() ;

        Dictionary options ;

        while ( res ) {
            options.insert({res.get<string>(0), res.get<string>(1)}) ;
            res.next() ;
        }

        return options ;
    } ;

    select("parent", fetcher).label("Parent:").required().addValidator([&] (const string &val, FormField &f) {

        int parent = stoi(val) ;
        if ( parent > 0 ){
            sqlite::Query q(con_, "SELECT name FROM menus WHERE id = ?", parent) ;
            sqlite::QueryResult res = q.exec() ;

            if ( !res ) {
                f.addErrorMsg("No such parent menu") ;
                return false ;
            }
            else return true ;
        }

    }) ;

    input("link", "text").label("Link:") ;

    checkbox("check", true).label("Options") ;
}
