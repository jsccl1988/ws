#include <wspp/util/sqlite/connection.hpp>
#include <wspp/util/sqlite/exception.hpp>
#include <wspp/util/sqlite/transaction.hpp>


using namespace std ;

namespace wspp { namespace util { namespace sql {

Connection::Connection(): handle_(nullptr) {
}

Connection::Connection(const std::string &name, int flags): Connection() {
    open(name, flags) ;
}

void Connection::open(const std::string &db, int flags) {
    if ( sqlite3_open_v2(db.c_str(), &handle_, flags, NULL)  != SQLITE_OK )
         throw Exception("Could not open database");
}

void Connection::close() {
    check() ;

    if ( sqlite3_close(handle_) != SQLITE_OK )
         throw Exception(sqlite3_errmsg(handle_));

    handle_ = nullptr ;
}

void Connection::exec(const string &sql, ...)
{
    va_list arguments ;
    va_start(arguments, sql);

    char *sql_ = sqlite3_vmprintf(sql.c_str(), arguments) ;

    char *err_msg ;
    if ( sqlite3_exec(handle_, sql_, NULL, NULL, &err_msg) != SQLITE_OK ) {
        string msg(err_msg) ;
        sqlite3_free(err_msg) ;

        throw Exception(msg) ;
    }

    sqlite3_free(sql_) ;

    va_end(arguments);
}

Statement Connection::prepareStatement(const string &sql)
{
    return Statement(*this, sql) ;
}

Query Connection::prepareQuery(const string &sql) {
    return Query(*this, sql) ;
}

Transaction Connection::transaction() {
    return Transaction(*this) ;
}

void Connection::check() {
    if( !handle_ )
        throw Exception("Database is not open.");
}

Connection::~Connection() {
    close() ;
}


} // namespace sqlite
} // namespace util
} // namespace wspp
