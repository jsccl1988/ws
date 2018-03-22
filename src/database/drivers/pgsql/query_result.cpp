#include "query_result.hpp"
#include "exceptions.hpp"

#include <wspp/database/types.hpp>

#include <boost/format.hpp>

namespace wspp {
namespace db {


PQQueryResultHandle::PQQueryResultHandle(const std::shared_ptr<wspp::db::PQStatementHandle> &stmt): stmt_(stmt) {

//    int num_fields = sqlite3_column_count(stmt_->handle());

//    for( int index = 0; index < num_fields; index++ ) {
//        const char* field_name = sqlite3_column_name(stmt_->handle(), index);
//        column_map_[field_name] = index ;
//    }
}

void PQQueryResultHandle::reset() {
//    pos_ = -1 ;
//    sqlite3_reset(stmt_->handle()) ;
}

void PQQueryResultHandle::check_has_row() const
{
    if ( pos_ < 0 )
        throw Exception("No current row in result") ;
}

bool PQQueryResultHandle::next() {

}

int PQQueryResultHandle::columns() const  {
}

int PQQueryResultHandle::columnType(int idx) const {

}

std::string PQQueryResultHandle::columnName(int idx) const  {
}

int PQQueryResultHandle::columnIndex(const std::string &name) const {
    check_has_row() ;
}

bool PQQueryResultHandle::columnIsNull(int idx) const {
    check_has_row() ;

}

void PQQueryResultHandle::read(int idx, int &val) const
{
    check_has_row() ;

}

void PQQueryResultHandle::read(int idx, unsigned int &val) const {
    check_has_row() ;

}

void PQQueryResultHandle::read(int idx, short int &val) const {
    check_has_row() ;

}

void PQQueryResultHandle::read(int idx, unsigned short &val) const {
    check_has_row() ;

}

void PQQueryResultHandle::read(int idx, long int &val) const {
    check_has_row() ;

}

void PQQueryResultHandle::read(int idx, unsigned long &val) const {
    check_has_row() ;

}

void PQQueryResultHandle::read(int idx, bool &v) const {
    check_has_row() ;

}

void PQQueryResultHandle::read(int idx, double &val) const {
    check_has_row() ;
}

void PQQueryResultHandle::read(int idx, float &val) const {
    check_has_row() ;
}

void PQQueryResultHandle::read(int idx, long long int &val) const {
    check_has_row() ;
}

void PQQueryResultHandle::read(int idx, unsigned long long &val) const {
    check_has_row() ;
}

void PQQueryResultHandle::read(int idx, std::string &val) const {
    check_has_row() ;
}

void PQQueryResultHandle::read(int idx, Blob &blob) const {
    check_has_row() ;
}

}
}


