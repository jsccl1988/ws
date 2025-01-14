#include "driver.hpp"
#include "connection.hpp"

using namespace std;
namespace wspp {
namespace db {
ConnectionHandlePtr SQLiteDriver::open(const util::Dictionary &params) const {
    sqlite3 *handle;

    int flags = 0;
    string database = params.get("db");

    if ( database.empty() ) return nullptr;

    string mode = params.get("mode", "rw");

    if ( mode == "rw")
        flags |= SQLITE_OPEN_READWRITE;
    else if ( mode == "r" )
        flags |= SQLITE_OPEN_READONLY;
    else if ( mode == "rc")
    flags |= SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

    if ( sqlite3_open_v2(database.c_str(), &handle, flags, NULL)  != SQLITE_OK )
        return nullptr;
    else
        return ConnectionHandlePtr(new SQLiteConnectionHandle(handle));
}
}
}