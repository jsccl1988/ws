#include "menu_controller.hpp"

#include <wspp/views/forms.hpp>
#include <wspp/views/table.hpp>

#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace wspp::web;
using namespace wspp::util;

// CREATE TABLE menus (id INTEGER PRIMARY KEY AUTOINCREMENT, name UNIQUE NOT NULL, parent INTEGER DEFAULT NULL, link TEXT DEFAULT NULL);
class MenusView: public SQLiteTableView {
public:
    MenusView(Connection &con): SQLiteTableView(con, "menus_list_view")  {
        con_.exec("CREATE TEMPORARY VIEW menus_list_view AS SELECT t1.id as id, t1.name as name, t1.link as link, t2.name as parent FROM menus AS t1 LEFT JOIN menus AS t2 ON t1.parent = t2.id");

        addColumn("Name", "name");
        addColumn("Parent", "parent");
        addColumn("Link", "link");
    }
};

void MenuController::fetch(){
    if ( !user_.check() ) {
        response_.stock_reply(Response::unauthorized);
        return;
    }

    MenusView view(con_);
    uint offset = request_.GET_.value<int>("page", 1);
    uint results_per_page = request_.GET_.value<int>("total", 10);

    Variant data = view.fetch(offset, results_per_page );
    response_.write(engine_.render("table-view", data ));
}

void MenuController::edit(){
    if ( !user_.check() ) {
        response_.stock_reply(Response::unauthorized);
        return;
    }

    Variant ctx( Variant::Object{
                     {"page_title", Variant::Object{
                          { "title", "Edit Menu" },
                        }
                     },
                     {"nav_brand", "blog"},
                     {"logged_in", user_.check()},
                     {"user_name", user_.userName()}});

    response_.write(engine_.render("menu-edit", ctx));
}

void MenuController::create(){
    if ( !user_.check() ) {
        response_.stock_reply(Response::unauthorized);
        return;
    }

    MenuForm form(con_);
    if ( request_.method_ == "POST" ) {
        if ( form.validate(request_.POST_) ) {
            // write data to database
            sqlite::Statement stmt(con_, "INSERT INTO menus ( name, parent, link ) VALUES ( ?, ?, ? )");

            stmt.bind(1, form.getValue("name"));
            stmt.bind(2, form.getValue("parent"));
            string link = form.getValue("link");
            if ( link.empty() ) stmt.bind(3, sqlite::Nil);
            else stmt.bind(3, link);
            stmt.exec();

            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}});
        } else {
            Variant ctx( Variant::Object{{"form", form.data()}} );

            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("menu-edit-dialog-new", ctx)}});
        }
    } else {
        Variant ctx( Variant::Object{{"form", form.data()}} );
        response_.write(engine_.render("menu-edit-dialog-new", ctx));
    }
}

void MenuController::update(){
    if ( !user_.check() ) {
        response_.stock_reply(Response::unauthorized);
        return;
    }

    MenuForm form(con_);
    if ( request_.method_ == "POST" ) {
        if ( form.validate(request_.POST_) ) {
            // write data to database
            sqlite::Statement stmt(con_, "UPDATE menus SET name = ?, parent = ?, link = ? WHERE id = ?");

            stmt.bind(1, form.getValue("name"));
            stmt.bind(2, form.getValue("parent"));
            string link = form.getValue("link");
            if ( link.empty() ) stmt.bind(3, sqlite::Nil);
            else stmt.bind(3, link);
            stmt.bind(4, request_.POST_.get("id"));
            stmt.exec();

            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}});
        } else {
            Variant ctx( Variant::Object{{"form", form.data()}} );
            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("@menu-edit-dialog-new.mst", ctx)}});
        }
    } else {
        const Dictionary &params = request_.GET_;
        string id = params.get("id");

        if ( id.empty() ) {
            response_.stock_reply(Response::not_found);
            return;
        }

        sqlite::Query q(con_, "SELECT * FROM menus WHERE id = ? LIMIT 1", id);
        sqlite::QueryResult res = q.exec();
        if ( !res ) {
            response_.stock_reply(Response::not_found);
            return;
        }

        form.init(res.getAll());

        Variant ctx( Variant::Object{{"form", form.data()}} );
        response_.write(engine_.render("menu-edit-dialog-new", ctx));
    }
}

void MenuController::remove(){
    if ( !user_.check() ) {
        response_.stock_reply(Response::unauthorized);
        return;
    }

    const Dictionary &params = request_.POST_;
    string id = params.get("id");

    if ( id.empty() )
        response_.stock_reply(Response::not_found);
    else {
        sqlite::Statement stmt(con_, "DELETE FROM menus where id=?", id);
        stmt.exec();

        response_.writeJSON("{}");
    }
}

class MenuNamesModel: public OptionsModel {
public:
    MenuNamesModel(Connection &con): con_(con) {}
    Dictionary fetch() override {
        sqlite::Query q(con_, "SELECT id, name FROM menus;");
        sqlite::QueryResult res = q.exec();

        Dictionary options;
        while ( res ) {
            options.insert({res.get<string>(0), res.get<string>(1)});
            res.next();
        }

        return options;
    }

private:
    Connection &con_;
};

MenuForm::MenuForm(sqlite::Connection &con): con_(con) {
    input("name", "text").label("Name:").required().addValidator([&] (const string &val, FormField &f) {
        sqlite::Query q(con_, "SELECT count(*) FROM menus WHERE name = ?", val);
        sqlite::QueryResult res = q.exec();

        if ( res.get<int>(0) ) {
            f.addErrorMsg("A menu with this name already exists");
            return false;
        }
        return true;
    });

    select("parent", boost::make_shared<MenuNamesModel>(con_)).label("Parent:").required().addValidator([&] (const string &val, FormField &f) {
        int parent = stoi(val);
        if ( parent > 0 ){
            sqlite::Query q(con_, "SELECT name FROM menus WHERE id = ?", parent);
            sqlite::QueryResult res = q.exec();

            if ( !res ) {
                f.addErrorMsg("No such parent menu");
                return false;
            } else {
                return true;
            }
        }
    });

    input("link", "text").label("Link:");
}
