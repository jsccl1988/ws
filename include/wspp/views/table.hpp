#ifndef __WSPP_UTIL_TABLE_VIEW_HPP__
#define __WSPP_UTIL_TABLE_VIEW_HPP__

#include <wspp/util/variant.hpp>
#include <wspp/util/database.hpp>
#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>

#include <string>
#include <vector>


using wspp::server::Request ;
using wspp::server::Response ;

namespace wspp {
namespace web {

class TemplateRenderer ;

using namespace util ;
// abstrcation of a table view

class TableView {
public:

    TableView() {}

    void setTitle(const std::string &title) { title_ = title ; }

    void addColumn(const std::string &header, const std::string &widget) {
        columns_.emplace_back(header, widget) ;
    }

    // return total records
    virtual uint count() = 0 ;

    // return count rocords starting at offset
    virtual Variant rows(uint offset, uint count) = 0 ;

    // fetch data to pass to the template renderer
    Variant::Object fetch(uint page, uint results_per_page);

    // a hook to modify the display value of a cell
    virtual Variant transform(const std::string &key, const std::string &value) { return value ; }

    void render(const Request &request, Response &response, TemplateRenderer &engine) ;

protected:

    struct Column {
    public:

        Column(const std::string &header, const std::string &widget): header_(header), widget_(widget) {}

        std::string header_, widget_ ;
    };


    std::vector<Column> columns_ ;
    std::string title_ ;

};

// Table view based on an SQlite database table

class SQLiteTableView: public TableView {
public:

    // Each row is populated by performing a select on the table, recovering the column names and associates values
    // For complex tables (e.g. with joins) subclass and create an SQL view in the constructor, passing the name of
    // the view to the base class. Note that the id should always be returned
    SQLiteTableView(sqlite::Connection &con, const std::string &table, const std::string &id_column = "id");

    Variant rows(uint offset, uint count) override;

    uint count() override {
        return con_.query("SELECT count(*) FROM " + table_)[0].as<uint>() ;
    }

protected:
    sqlite::Connection &con_ ;
    std::string table_, id_column_ ;
};



}
}




#endif
