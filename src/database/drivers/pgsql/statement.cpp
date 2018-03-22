#include "statement.hpp"
#include "exceptions.hpp"
#include "query_result.hpp"

#include <cstring>

using namespace std ;

namespace wspp { namespace db {


void PQStatementHandle::check() const {
    if ( !handle_ )
        throw Exception("Statement has not been compiled.");
}

void PQStatementHandle::clear() {

}

void PQStatementHandle::finalize()
{
}


StatementHandle &PQStatementHandle::bind(int idx, const NullType &) {

    check();
//    if ( sqlite3_bind_null(handle_, idx) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}


StatementHandle &PQStatementHandle::bind(int idx, unsigned char v) {
    check();
//    if ( sqlite3_bind_int(handle_, idx, v) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, char v) {
    check();
//    if ( sqlite3_bind_int(handle_, idx, v) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, int v) {
    check();
//    if ( sqlite3_bind_int(handle_, idx, v) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, unsigned int v) {
    check();
//    if ( sqlite3_bind_int(handle_, idx, v) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, unsigned short int v) {
    check();
//    if ( sqlite3_bind_int(handle_, idx, v) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, short int v) {
    check();
//    if ( sqlite3_bind_int(handle_, idx, v) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, long int v) {
    check();
//    if ( sqlite3_bind_int64(handle_, idx, v) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, unsigned long int v) {
    check();
//    if ( sqlite3_bind_int64(handle_, idx, v) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, long long int v){
    check();
//    if ( sqlite3_bind_int64(handle_, idx, v) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, unsigned long long int v){
    check();
//    if ( sqlite3_bind_int64(handle_, idx, v) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, double v){
    check() ;
//    if ( sqlite3_bind_double(handle_, idx, v) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, float v){
    check() ;
//    if ( sqlite3_bind_double(handle_, idx, v) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, const string &v){
    check() ;
//    if ( sqlite3_bind_text(handle_, idx, v.c_str(), int(v.size()), SQLITE_TRANSIENT ) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, const Blob &blob){
    check() ;
//    if ( sqlite3_bind_blob(handle_, idx, blob.data(), blob.size(), SQLITE_TRANSIENT ) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

StatementHandle &PQStatementHandle::bind(int idx, const char *v){
    check() ;
//    if ( sqlite3_bind_text(handle_, idx, v, strlen(v), SQLITE_TRANSIENT ) != SQLITE_OK )
//        throw PQException(sqlite3_db_handle(handle_));
    return *this ;
}

int PQStatementHandle::placeholderNameToIndex(const std::string &name) {
//    int idx = sqlite3_bind_parameter_index(handle_, name.c_str() );
//    if ( idx ) return idx ;
//    else throw Exception(name + " is not a valid statement placeholder") ;
}

void PQStatementHandle::exec()
{

        check() ;

  //      sqlite3_step(handle_) ;

}

QueryResult PQStatementHandle::execQuery()
{

    //return QueryResult(QueryResultHandlePtr(new SQLiteQueryResultHandle(shared_from_this()))) ;
}

} // namespace db
} // namespace wspp
