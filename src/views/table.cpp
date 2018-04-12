#include <wspp/views/table.hpp>
#include <wspp/twig/renderer.hpp>

#include <cmath>

using namespace std ;
using namespace wspp::twig ;

namespace wspp {
namespace web {

Variant::Object TableView::fetch(uint page, uint results_per_page) {

    // get number of records

    uint total_count = count() ;

    uint num_pages = ceil(total_count/(double)results_per_page) ;

    if ( page > num_pages ) page = 1 ;

    uint offset = (page - 1) * results_per_page ;

    Variant entries = rows(offset, results_per_page) ;

    return Variant::Object({ {"page", page}, {"rows", entries}, {"total_rows", total_count}, {"total_pages", num_pages }} ) ;
}

void TableView::handle(const server::Request &request, server::Response &response) {
    uint offset = request.GET_.value<int>("page", 1) ;
    uint results_per_page = request.GET_.value<int>("total", 10) ;

    const Variant::Object &data = fetch(offset, results_per_page) ;

    response.writeJSONVariant(data) ;
}

SQLTableView::SQLTableView(Connection &con, const string &table, const string &id_column):
    TableView(), con_(con), table_(table), id_column_(id_column) {
}

Variant SQLTableView::rows(uint offset, uint count)  {

    ostringstream sql ;
    sql << "SELECT * FROM " << '"' <<  table_ << '"' << " LIMIT ?, ?" ;

    Variant::Array entries ;

    for( auto &&r: con_.query(sql.str(), offset, count)) {
        Variant::Object row ;

        string id = r[id_column_].as<string>() ;

        for( uint i=0 ; i<r.columns() ; i++ ) {
            string key = r.columnName(i) ;
            row.insert({{key, transform(key, r[i].as<string>())}}) ;
        }

        entries.emplace_back(Variant::Object{{"id", id}, {"data", row}}) ;

    }


    return entries ;
}


} // namespace web

} // namespace wspp
