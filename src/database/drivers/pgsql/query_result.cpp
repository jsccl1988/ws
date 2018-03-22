#include "query_result.hpp"
#include "exceptions.hpp"

#include <wspp/database/types.hpp>

#include <boost/format.hpp>

namespace wspp {
namespace db {


PGSQLQueryResultHandle::PGSQLQueryResultHandle(const std::shared_ptr<wspp::db::PGSQLStatementHandle> &stmt): stmt_(stmt) {

//    int num_fields = sqlite3_column_count(stmt_->handle());

//    for( int index = 0; index < num_fields; index++ ) {
//        const char* field_name = sqlite3_column_name(stmt_->handle(), index);
//        column_map_[field_name] = index ;
//    }
}

void PGSQLQueryResultHandle::reset() {
//    pos_ = -1 ;
//    sqlite3_reset(stmt_->handle()) ;
}

void PGSQLQueryResultHandle::check_has_row() const
{
    if ( pos_ < 0 )
        throw Exception("No current row in result") ;
}

bool PGSQLQueryResultHandle::next() {

}

int PGSQLQueryResultHandle::columns() const  {
}

int PGSQLQueryResultHandle::columnType(int idx) const {

}

std::string PGSQLQueryResultHandle::columnName(int idx) const  {
}

int PGSQLQueryResultHandle::columnIndex(const std::string &name) const {
    check_has_row() ;
}

bool PGSQLQueryResultHandle::columnIsNull(int idx) const {
    check_has_row() ;

}

void PGSQLQueryResultHandle::read(int idx, int &val) const
{
    check_has_row() ;

}

void PGSQLQueryResultHandle::read(int idx, unsigned int &val) const {
    check_has_row() ;

}

void PGSQLQueryResultHandle::read(int idx, short int &val) const {
    check_has_row() ;

}

void PGSQLQueryResultHandle::read(int idx, unsigned short &val) const {
    check_has_row() ;

}

void PGSQLQueryResultHandle::read(int idx, long int &val) const {
    check_has_row() ;

}

void PGSQLQueryResultHandle::read(int idx, unsigned long &val) const {
    check_has_row() ;

}

void PGSQLQueryResultHandle::read(int idx, bool &v) const {
    check_has_row() ;

}

void PGSQLQueryResultHandle::read(int idx, double &val) const {
    check_has_row() ;
}

void PGSQLQueryResultHandle::read(int idx, float &val) const {
    check_has_row() ;
}

void PGSQLQueryResultHandle::read(int idx, long long int &val) const {
    check_has_row() ;
}

void PGSQLQueryResultHandle::read(int idx, unsigned long long &val) const {
    check_has_row() ;
}

void PGSQLQueryResultHandle::read(int idx, std::string &val) const {
    check_has_row() ;
}

void PGSQLQueryResultHandle::read(int idx, Blob &blob) const {
    check_has_row() ;
}

}
}


