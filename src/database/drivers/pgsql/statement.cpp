#include "statement.hpp"
#include "exceptions.hpp"
#include "query_result.hpp"

#include <cstring>

using namespace std ;

namespace wspp { namespace db {


void PGSQLStatementHandle::check() const {
    if ( !handle_ )
        throw Exception("Statement has not been compiled.");
}

void PGSQLStatementHandle::clear() {

}

void PGSQLStatementHandle::finalize()
{
}


StatementHandle &PGSQLStatementHandle::bind(int idx, const NullType &v) {
    check();
    params_.add(idx, v) ;
    return *this ;
}


StatementHandle &PGSQLStatementHandle::bind(int idx, unsigned char v) {
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, char v) {
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, int v) {
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, unsigned int v) {
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, unsigned short int v) {
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, short int v) {
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, long int v) {
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, unsigned long int v) {
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, long long int v){
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, unsigned long long int v){
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, double v){
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, float v){
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, const string &v){
    check();
    params_.add(idx, v) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, const Blob &blob){
    check();
    params_.add(idx, blob) ;
    return *this ;
}

StatementHandle &PGSQLStatementHandle::bind(int idx, const char *v){
    check();
    params_.add(idx, v) ;
    return *this ;
}

int PGSQLStatementHandle::placeholderNameToIndex(const std::string &name) {

}

void PGSQLStatementHandle::exec()
{

        check() ;

        vector<const char *> values ;
        vector<int> lengths, binaries ;
        params_.marshall(values, lengths, binaries) ;
  //      sqlite3_step(handle_) ;

}

QueryResult PGSQLStatementHandle::execQuery()
{

    //return QueryResult(QueryResultHandlePtr(new SQLiteQueryResultHandle(shared_from_this()))) ;
}

} // namespace db
} // namespace wspp
