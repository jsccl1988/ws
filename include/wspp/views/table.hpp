#ifndef __WSPP_UTIL_TABLE_VIEW_HPP__
#define __WSPP_UTIL_TABLE_VIEW_HPP__

#include <wspp/util/variant.hpp>
#include <wspp/util/database.hpp>

#include <string>
#include <vector>

namespace wspp {
namespace web {

using namespace util ;
// abstrcation of a table view

class TableView {
public:

    TableView() {}

    void addColumn(const std::string &header, const std::string &name_key, const std::string &col_type = std::string()) {
        columns_.emplace_back(header, name_key, col_type) ;
    }

    // return total records
    virtual uint count() = 0 ;

    // return count rocords starting at offset
    virtual Variant rows(uint offset, uint count) = 0 ;

    // fetch data to pass to the template renderer
    Variant::Object fetch(uint page, uint results_per_page);

protected:

    struct Column {
    public:

        Column(const std::string &header, const std::string &name, const std::string &value = std::string()): header_(header), key_(name), type_(value) {}

        std::string key_, header_, type_ ;
    };


    std::vector<Column> columns_ ;
};

// Table view based on an SQlite database table

class SQLiteTableView: public TableView {
public:
    SQLiteTableView(sqlite::Connection &con, const std::string &table, const std::string &id_column = "id"):
        con_(con), table_(table), id_column_(id_column) {
    }

    Variant rows(uint offset, uint count) override;

    uint count() override {
        sqlite::Query stmt(con_, "SELECT count(*) FROM " + table_) ;
        sqlite::QueryResult res = stmt.exec() ;
        return res.get<int>(0) ;
    }

protected:
    sqlite::Connection &con_ ;
    std::string table_, id_column_ ;
};



}
}




#endif
