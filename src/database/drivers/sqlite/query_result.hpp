#ifndef __SQLITE_QUERY_RESULT_HANDLE_HPP__
#define __SQLITE_QUERY_RESULT_HANDLE_HPP__

#include <wspp/database/query_result_handle.hpp>
#include <wspp/database/types.hpp>

#include <sqlite3.h>

namespace wspp {
namespace db {

class SQLiteQueryResultHandle: public QueryResultHandle {
public:
    SQLiteQueryResultHandle(sqlite3_stmt *stmt): stmt_(stmt) {}

    ~SQLiteQueryResultHandle() {}

    int at() const override {
        return pos_ ;
    }


    bool next() override ;

    int columns() const override ;

    int columnType(int idx) const override ;

    std::string columnName(int idx) const override ;

    int columnIndex(const std::string &name) const override ;

    void read(int idx, int &val) const override ;
    void read(int idx, unsigned int &val) const override ;
    void read(int idx, short int &val) const override ;
    void read(int idx, unsigned short int &val) const override ;
    void read(int idx, long int &val) const override ;
    void read(int idx, unsigned long int &val) const override ;
    void read(int idx, bool &val) const override ;
    void read(int idx, double &val) const override ;
    void read(int idx, float &val) const override ;
    void read(int idx, long long int &val) const override ;
    void read(int idx, unsigned long long int &val) const override ;
    void read(int idx, std::string &val) const override ;
    void read(int idx, Blob &val) const override ;

    void reset() override;
private:

    sqlite3_stmt *stmt_ ;
    int pos_ = -1 ;
} ;

}
}

#endif
