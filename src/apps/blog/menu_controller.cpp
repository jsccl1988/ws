#include "menu_controller.hpp"
#include <wspp/util/forms.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>

using namespace std ;
using namespace wspp ;

// CREATE TABLE menus (id INTEGER PRIMARY KEY AUTOINCREMENT, name UNIQUE NOT NULL, parent INTEGER DEFAULT NULL, link TEXT DEFAULT NULL);


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

struct Column {
    Column(const string &header, const string &name): header_(header), name_(name) {

    }

    string name_ ;
    string header_ ;

};


static Variant makePagerData(uint page, uint max_page, const string url_prefix)
{
    Variant::Array pages ;
    if ( max_page == 1 ) return pages ;

    // Show pager

    int delta = 4 ;

    int min_surplus = (page <= delta) ? (delta - page + 1) : 0;
    int max_surplus = (page >= (max_page - delta)) ?
                       (page - (max_page - delta)) : 0;

    int start =  std::max<int>(page - delta - max_surplus, 1) ;
    int stop = std::min(page + delta + min_surplus, max_page) ;

    // if ( start > 1 ) $nav .= '<li>...</li>' ;


    if ( page > 1 )
    {
        uint p = page - 1 ;

        pages.emplace_back(Variant::Object{{"key", p}, {"text", "Previews" }});
        pages.emplace_back(Variant::Object{{"key", 1}, {"text", "First" }});
    }

    for( uint p = start ; p <= stop ; p++ )
    {
        if ( p == page ) pages.emplace_back(Variant::Object{ {"active", true }, {"text", p }}); // no need to create a link to current page
        else {
            pages.emplace_back(Variant::Object{{"key", p}, {"text", p }}); // no need to create a link to current page
        }
    }

    if ( page < max_page )
    {
        uint p = page + 1 ;

        pages.emplace_back(Variant::Object{{"key", p}, {"text", "Next"}});
        pages.emplace_back(Variant::Object{{"key", max_page}, {"text", "Last"}});
    }

    return pages ;
}

class TableView {
public:

    TableView() {}

    void addColumn(const string &header, const string &name) {
        columns_.emplace_back(header, name) ;
    }

    virtual uint count() = 0 ;
    virtual Variant rows(uint offset, uint count) = 0 ;

    Variant fetch(uint page, uint results_per_page) {

        // get number of records

        uint total_count = count() ;


        Variant::Array headers ;
        for( const Column &c: columns_ ) {
            headers.push_back(c.header_)    ;
        }

        uint num_pages = ceil(total_count/(double)results_per_page) ;

        uint offset = (page - 1) * results_per_page ;

        Variant entries = rows(offset, results_per_page) ;

        Variant pages = makePagerData(page, num_pages, "/page/list/%%page%%/") ;

        return Variant::Object({{"pages", pages}, {"headers", headers}, {"rows", entries}, {"total_rows", total_count}, {"total_pages", num_pages }} ) ;
    }

    vector<Column> columns_ ;

};


class MenusView: public TableView {
public:
    MenusView(Connection &con): con_(con) {
        addColumn("Name", "name") ;
        addColumn("Parent", "parent") ;
        addColumn("Link", "link") ;
    }

    Variant rows(uint offset, uint count) override {

        sqlite::Query q(con_, "SELECT t1.name as name, t1.link as link, t2.name as parent FROM menus AS t1 LEFT JOIN menus AS t2 ON t1.parent = t2.id LIMIT ?, ?", offset, count) ;
        sqlite::QueryResult res = q.exec() ;

        Variant::Array entries ;

        while ( res ) {
            Variant::Array columns ;

            for( const Column &c: columns_ ) {
                string cname = c.name_ ;
                if ( res.hasColumn(cname) ) {
                    columns.emplace_back(Variant(res.get<string>(cname))) ;
                }
            }

            entries.emplace_back(Variant::Object{{"columns", columns}}) ;
;
            res.next() ;
        };

        return entries ;

    }

    uint count() override {
        sqlite::Query stmt(con_, "SELECT count(*) FROM menus") ;
        sqlite::QueryResult res = stmt.exec() ;
        return res.get<int>(0) ;
    }

private:
    Connection &con_ ;
};

void MenuController::fetch()
{
    if ( !user_.isLoggedIn() ) {
        response_.stock_reply(Response::unauthorized) ;
        return ;
    }

    MenusView view(con_) ;
    uint offset = request_.GET_.value<int>("page", 1) ;
    uint results_per_page = request_.GET_.value<int>("total", 10) ;

    Variant data = view.fetch(offset, results_per_page ) ;

    response_.write(engine_.render("@menu-edit-list.mst", data )) ;
}



void MenuController::edit()
{
    if ( !user_.isLoggedIn() ) {
        response_.stock_reply(Response::unauthorized) ;
        return ;
    }

    Variant ctx( Variant::Object{
                     {"page_title", Variant::Object{
                          { "title", "Edit Menu" },
                      }
                     },
                     {"menus", Variant::Object{{"list", fetchList()}}},
                     {"nav_brand", "blog"},
                     {"logged_in", user_.isLoggedIn()},
                     {"user_name", user_.name()}}) ;

    response_.write(engine_.render("@menu-edit.mst", ctx)) ;
}

void MenuController::create()
{
    if ( !user_.isLoggedIn() ) {
        response_.stock_reply(Response::unauthorized) ;
        return ;
    }

    MenuForm form(con_) ;

    if ( request_.method_ == "POST" ) {

        if ( form.validate(request_.POST_) ) {

            // write data to database

            sqlite::Statement stmt(con_, "INSERT INTO menus ( name, parent, link ) VALUES ( ?, ?, ? )") ;

            stmt.bind(1, form.getValue("name")) ;
            stmt.bind(2, form.getValue("parent")) ;
            string link = form.getValue("link") ;
            if ( link.empty() ) stmt.bind(3, sqlite::Nil) ;
            else stmt.bind(3, link) ;
            stmt.exec() ;

            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}}) ;
        }
        else {
            Variant ctx( Variant::Object{{"menus_form", form.data()}} ) ;

            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("@menu-edit-dialog-new.mst", ctx)}});
        }
    }
    else {
        Variant ctx( Variant::Object{{"menus_form", form.data()}} ) ;

        response_.write(engine_.render("@menu-edit-dialog-new.mst", ctx)) ;
    }
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

class MenuNamesModel: public OptionsModel {
public:
    MenuNamesModel(Connection &con): con_(con) {}
    Dictionary fetch() override {
        sqlite::Query q(con_, "SELECT id, name FROM menus;") ;
        sqlite::QueryResult res = q.exec() ;

        Dictionary options ;

        while ( res ) {
            options.insert({res.get<string>(0), res.get<string>(1)}) ;
            res.next() ;
        }

        return options ;
    }

private:
    Connection &con_ ;
};

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

    } ;

    select("parent", boost::make_shared<MenuNamesModel>(con_)).label("Parent:").required().addValidator([&] (const string &val, FormField &f) {

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
