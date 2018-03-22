#include "connection.hpp"
#include "exceptions.hpp"
#include "statement.hpp"

using namespace std ;

namespace wspp {
namespace db {

void PQConnectionHandle::close() {
    PQfinish(handle_) ;
}

StatementHandlePtr PQConnectionHandle::createStatement(const std::string &sql)
{
    /*
    const char * tail = 0;

    sqlite3_stmt *stmt ;
    if ( sqlite3_prepare_v2(handle_, sql.c_str(), -1, &stmt ,&tail) != SQLITE_OK )
        throw SQLiteException(handle_) ;

    return StatementHandlePtr(new SQLiteStatementHandle(stmt)) ;
    */
}


void PQConnectionHandle::exec(const char *sql)
{
    PGresult *res = PQexec(handle_, sql);

    if ( PQresultStatus(res) != PGRES_COMMAND_OK ) {
        PQclear(res);
        throw PQException(handle_) ;
    }

    PQclear(res);
}


void PQConnectionHandle::begin() {
    exec("BEGIN") ;
}

void PQConnectionHandle::commit() {
    exec("COMMIT") ;
}

void PQConnectionHandle::rollback() {
    exec("ROLLBACK");
}

uint64_t PQConnectionHandle::last_insert_rowid() const
{
    //return sqlite3_last_insert_rowid(handle_) ;
    return 0 ;
}

}
}
