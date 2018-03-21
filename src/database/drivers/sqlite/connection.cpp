#include "connection.hpp"
#include "exceptions.hpp"
#include "statement.hpp"

namespace wspp {
namespace db {

void SQLiteConnectionHandle::close() {
    sqlite3_close(handle_) ;
}

StatementHandlePtr SQLiteConnectionHandle::createStatement(const std::string &sql)
{
    const char * tail = 0;

    sqlite3_stmt *stmt ;
    if ( sqlite3_prepare_v2(handle_, sql.c_str(), -1, &stmt ,&tail) != SQLITE_OK )
        throw SQLiteException(handle_) ;

    return StatementHandlePtr(new SQLiteStatementHandle(stmt)) ;
}


}
}
