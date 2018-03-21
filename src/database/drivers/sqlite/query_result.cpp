#include "query_result.hpp"
#include "exceptions.hpp"

#include <wspp/database/types.hpp>

#include <boost/format.hpp>

namespace wspp {
namespace db {


void SQLiteQueryResultHandle::reset() {
    pos_ = -1 ;
    sqlite3_reset(stmt_) ;
}

bool SQLiteQueryResultHandle::next() {
    if ( pos_ == -2 )
        throw Exception("next called passed the end of the record set");

    switch ( sqlite3_step(stmt_) ) {
    case SQLITE_ROW:
        pos_ ++ ;
        return true ;
    case SQLITE_DONE:
        pos_ = -2 ;
        return false ;
    default:
        throw SQLiteException(sqlite3_db_handle(stmt_));
    }
}

int SQLiteQueryResultHandle::columns() const  {
     return ( sqlite3_data_count(stmt_) ) ;
}

int SQLiteQueryResultHandle::columnType(int idx) const {
    return sqlite3_column_type(stmt_, idx);
}

std::string SQLiteQueryResultHandle::columnName(int idx) const  {
    const char *name = sqlite3_column_name(stmt_, idx)  ;
    if ( name == nullptr ) throw Exception(str(boost::format("There is no column with index %d") % idx)) ;
    else return name ;
}

int SQLiteQueryResultHandle::columnIndex(const std::string &name) const {

}

void SQLiteQueryResultHandle::read(int idx, int &val) const
{
    val = sqlite3_column_int(stmt_, idx);
}

void SQLiteQueryResultHandle::read(int idx, unsigned int &val) const {
    val = sqlite3_column_int(stmt_, idx);
}

void SQLiteQueryResultHandle::read(int idx, short int &val) const {
    val = sqlite3_column_int(stmt_, idx);
}

void SQLiteQueryResultHandle::read(int idx, unsigned short &val) const {
    val = sqlite3_column_int(stmt_, idx);
}

void SQLiteQueryResultHandle::read(int idx, long int &val) const {
    val = sqlite3_column_int64(stmt_, idx);
}

void SQLiteQueryResultHandle::read(int idx, unsigned long &val) const {
    val = sqlite3_column_int64(stmt_, idx);
}

void SQLiteQueryResultHandle::read(int idx, bool &v) const {
    v = sqlite3_column_int(stmt_, idx);
}

void SQLiteQueryResultHandle::read(int idx, double &val) const {
    val = sqlite3_column_double(stmt_, idx);
}

void SQLiteQueryResultHandle::read(int idx, float &val) const {
    val = sqlite3_column_double(stmt_, idx);
}

void SQLiteQueryResultHandle::read(int idx, long long int &val) const {
    val = sqlite3_column_int64(stmt_, idx);
}

void SQLiteQueryResultHandle::read(int idx, unsigned long long &val) const {
    val = sqlite3_column_int64(stmt_, idx);
}

void SQLiteQueryResultHandle::read(int idx, std::string &val) const {
    const char *res = reinterpret_cast<char const*>(sqlite3_column_text(stmt_, idx));
    if ( res == nullptr ) return  ;
    val.assign(res) ;
}

void SQLiteQueryResultHandle::read(int idx, Blob &blob) const {
    const void *data = sqlite3_column_blob(stmt_, idx);
    int bytes = sqlite3_column_bytes(stmt_, idx) ;
    blob = Blob((const char *)data, bytes) ;
}

}
}


