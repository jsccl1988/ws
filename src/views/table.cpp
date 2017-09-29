#include <wspp/views/table.hpp>

using namespace std ;

namespace wspp {
namespace web {

static Variant make_pager_data(uint page, uint max_page)
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


    pages.emplace_back(Variant::Object{{"first", Variant::Object{{"page", 1}} }});

    if ( page > 1 )
        pages.emplace_back(Variant::Object{{"previous", Variant::Object{{"page", page-1}} }});
    else
        pages.emplace_back(Variant::Object{{"previous", Variant::Object{{"disabled", true}} }});


    Variant::Array page_entries ;
    for( uint p = start ; p <= stop ; p++ )
    {
        if ( p == page ) page_entries.emplace_back(Variant::Object{ {"active", true }, {"page", p }, {"text", p }}); // no need to create a link to current page
        else {
            page_entries.emplace_back(Variant::Object{{"page", p}, {"text", p }}); // no need to create a link to current page
        }
    }

    pages.emplace_back(Variant::Object{{"pages", page_entries}}) ;

    if ( page < max_page )
        pages.emplace_back(Variant::Object{{"next", Variant::Object{{"page", page+1}} }});
    else
        pages.emplace_back(Variant::Object{{"next", Variant::Object{{"disabled", true}} }});

    pages.emplace_back(Variant::Object{{"last", Variant::Object{{"page", max_page}} }});

    return pages ;
}

Variant::Object TableView::fetch(uint page, uint results_per_page) {

    // get number of records

    uint total_count = count() ;

    Variant::Array headers ;
    for( const Column &c: columns_ ) {
        headers.push_back(Variant::Object{{"name", c.header_}, {"widget", c.widget_}})    ;
    }

    uint num_pages = ceil(total_count/(double)results_per_page) ;

    if ( page > num_pages ) page = 1 ;

    uint offset = (page - 1) * results_per_page ;

    Variant entries = rows(offset, results_per_page) ;

    Variant pages = make_pager_data(page, num_pages) ;

    return Variant::Object({{"page", page}, {"pager", pages}, {"headers", headers}, {"rows", entries}, {"total_rows", total_count}, {"total_pages", num_pages }} ) ;
}

SQLiteTableView::SQLiteTableView(sqlite::Connection &con, const string &table, const string &id_column):
    TableView(), con_(con), table_(table), id_column_(id_column) {

   /* ostringstream sql ;
    sql << "SELECT * FROM \"" << table_ << "\" LIMIT 0" ;
    sqlite::Query stmt(con_, sql.str()) ;
    sqlite::QueryResult res = stmt.exec() ;
    for(uint i=0 ; i<res.columns() ; i++ ) {
        keys_.emplace_back(string(res.columnName(i))) ;
    }
    */
}

Variant SQLiteTableView::rows(uint offset, uint count)  {

    ostringstream sql ;
    sql << "SELECT * FROM " << '"' <<  table_ << '"' << " LIMIT ?, ?" ;

    sqlite::Query q(con_, sql.str(), offset, count) ;
    sqlite::QueryResult res = q.exec() ;

    Variant::Array entries ;

    while ( res ) {
        Variant::Object row ;

        string id = res.get<string>(id_column_) ;

        for( uint i=0 ; i<res.columns() ; i++ ) {
            string key = res.columnName(i) ;
            row.insert({{key, transform(key, res.get<string>(i))}}) ;
        }

        entries.emplace_back(Variant::Object{{"id", id}, {"data", row}}) ;

        res.next() ;
    };

    return entries ;
}


} // namespace web

} // namespace wspp
