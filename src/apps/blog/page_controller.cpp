#include "page_controller.hpp"

#include <boost/algorithm/string.hpp>

using namespace std ;
using namespace wspp ;

// CREATE TABLE pages (id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT, content TEXT, permalink TEXT);

void PageController::create()
{
    if ( !user_.isLoggedIn() ) {
        response_.stock_reply(Response::unauthorized) ;
        return ;
    }


    Variant ctx( Variant::Object{
                 {"page_title", Variant::Object{
                            { "title", "Add new page" },
                          }
                 },
                 {"nav_brand", "blog"},
                 {"logged_in", user_.isLoggedIn()},
                 {"user_name", user_.name()}}) ;

    response_.write(engine_.render("page-edit", ctx)) ;
}

void PageController::publish()
{
    if ( !user_.isLoggedIn() ) {
        response_.stock_reply(Response::unauthorized) ;
        return ;
    }

    const Dictionary &params = request_.POST_ ;
    string title = params.get("title") ;
    string content = params.get("content") ;
    string permalink = params.get("permalink") ;
    string id = params.get("id") ;

    if ( id.empty() ) {
        sqlite::Statement stmt(con_, "INSERT INTO pages (title, content, permalink) VALUES (?, ?, ?)", title, content, permalink) ;
        stmt.exec() ;
        int id = con_.last_insert_rowid() ;
        string href = "/page/" + to_string(id) ;
        response_.writeJSONVariant(Variant::Object{{"id", to_string(id)}, {"msg", "Page succesfully created. View <a href=\"" + href + "\">page</a>"}}) ;
    }
    else {
        sqlite::Statement stmt(con_, "REPLACE INTO pages (id, title, content, permalink) VALUES (?, ?, ?, ?)", id, title, content, permalink) ;
        stmt.exec() ;
        string href = "/page/" + id ;
        response_.writeJSONVariant(Variant::Object{{"id", id}, {"msg", "Page succesfully updated. View <a href=\"" + href + "\">page</a>"}}) ;
    }
}

void PageController::edit(const string &id)
{
    if ( !user_.isLoggedIn() ) {
        response_.stock_reply(Response::unauthorized) ;
        return ;
    }

    sqlite::Query stmt(con_, "SELECT title, content, permalink FROM pages WHERE id=?", id) ;
    sqlite::QueryResult res = stmt.exec() ;

    if ( res ) {

        Variant ctx( Variant::Object{
                     {"page_title", "Add new page" },
                     {"nav_brand", "blog"},
                     {"logged_in", user_.isLoggedIn()},
                     {"user_name", user_.name()},
                     {"page", Variant::Object({
                          {"id", id},
                          {"title", res.get<string>("title")},
                          {"content", res.get<string>("content")},
                          {"permalink", res.get<string>("permalink")}
                         })
                     } }) ;


        response_.write(engine_.render("@page-edit.mst", ctx)) ;

    }
    else
        response_.stock_reply(Response::not_found) ;
}

void PageController::remove()
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
        sqlite::Statement stmt(con_, "DELETE FROM pages where id=?", id) ;
        stmt.exec() ;
    }

}

void PageController::show(const std::string &page_id)
{


    sqlite::Query q(con_, "SELECT title, content FROM pages WHERE id=? ;", page_id) ;
    sqlite::QueryResult res = q.exec() ;

    if ( res ) {
        Variant ctx( Variant::Object{
                     {"page", Variant::Object{
                                { "id", page_id },
                                { "title", res.get<string>("title") },
                                { "content", res.get<string>("content") }
                              }
                     },
                     {"nav_brand", "blog"},
                     {"logged_in", user_.isLoggedIn()},
                     {"user_name", user_.name()}}) ;
        response_.write(engine_.render("page", ctx)) ;
    }
    else
        response_.stock_reply(Response::not_found);

}


void makePagerData(Variant::Array &pages, uint page, uint max_page, const string url_prefix)
{
    if ( max_page == 1 ) return ;

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
        string url_prev = boost::replace_all_copy(url_prefix, "%%page%%", to_string(p)) ;
        string url_first = boost::replace_all_copy(url_prefix, "%%page%%", "1") ;


        pages.emplace_back(Variant::Object{{"href", url_prev}, {"text", "Previews" }});
        pages.emplace_back(Variant::Object{{"href", url_first}, {"text", "First" }});
    }

    for( uint p = start ; p <= stop ; p++ )
    {
        if ( p == page ) pages.emplace_back(Variant::Object{{"href", "#"}, {"active", true }, {"text", p }}); // no need to create a link to current page
        else {
            string url = boost::replace_all_copy(url_prefix, "%%page%%", to_string(p)) ;
            pages.emplace_back(Variant::Object{{"href", url}, {"text", p }}); // no need to create a link to current page
        }
    }

    if ( page < max_page )
    {
        uint p = page + 1 ;
        string url_next = boost::replace_all_copy(url_prefix, "%%page%%", to_string(p)) ;
        string url_last = boost::replace_all_copy(url_prefix, "%%page%%", to_string(max_page)) ;

        pages.emplace_back(Variant::Object{{"href", url_next}, {"text", "Next"}});
        pages.emplace_back(Variant::Object{{"href", url_last}, {"text", "Last"}});
    }

}

void PageController::list(uint view) {

    if ( !user_.isLoggedIn() ) {
        response_.stock_reply(Response::unauthorized) ;
        return ;
    }

    uint total_pages = 0 ;

    {
        sqlite::Query q(con_, "SELECT count(*) FROM pages") ;
        sqlite::QueryResult res = q.exec() ;
        if ( res ) total_pages = res.get<int>(0) ;
    }


    const uint max_pages_per_view = 1 ;

    uint num_views = ceil(total_pages/(double)max_pages_per_view) ;

    uint page_offset = (view - 1) * max_pages_per_view ;


    sqlite::Query q(con_, "SELECT id, title FROM pages LIMIT ?, ?;", page_offset, max_pages_per_view) ;
    sqlite::QueryResult res = q.exec() ;

    Variant::Array entries ;

    while ( res ) {
        Variant::Object page {
            { "id", res.get<string>("id") },
            { "title", res.get<string>("title") }
        } ;
        entries.emplace_back(page) ;
        res.next() ;
    };

    Variant::Array pages ;

    makePagerData(pages, view, num_views, "/page/list/%%page%%/") ;

    Variant ctx(Variant::Object{{"entries", entries},
                                {"pages", pages},
                                {"nav_brand", "blog"},
                                {"logged_in", user_.isLoggedIn()},
                                {"user_name", user_.name()}}) ;

    response_.write(engine_.render("pages", ctx)) ;

}
