#ifndef __DATABASE_SQLITE_DRIVER_CONNECTION_HPP__
#define __DATABASE_SQLITE_DRIVER_CONNECTION_HPP__

#include <sqlite3.h>

#include <wspp/database/connection_handle.hpp>

namespace wspp { namespace db {

class SQLiteConnectionHandle: public ConnectionHandle {
public:
    SQLiteConnectionHandle(sqlite3 *handle): handle_(handle) {}

    void close() override ;

    StatementHandlePtr createStatement(const std::string &sql) ;

private:

    sqlite3 *handle_ ;
};



} // namespace db
} // namespace wspp

#endif
