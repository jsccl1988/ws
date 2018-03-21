#include "driver.hpp"


#include "connection.hpp"

namespace wspp {
namespace db {

ConnectionHandlePtr SQLiteDriver::open(const std::string &params) const {
    sqlite3 *handle ;
    int flags = SQLITE_OPEN_READWRITE ;

    if ( sqlite3_open_v2(params.c_str(), &handle, flags, NULL)  != SQLITE_OK )
        return nullptr ;
    else
        return ConnectionHandlePtr(new SQLiteConnectionHandle(handle)) ;
}
}
}
