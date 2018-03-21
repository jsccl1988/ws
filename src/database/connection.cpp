#include <wspp/database/connection.hpp>
#include <wspp/database/exception.hpp>
#include <wspp/database/transaction.hpp>

#include <wspp/database/connection_handle.hpp>

#include "driver_factory.hpp"

using namespace std ;

namespace wspp { namespace db {

Connection::Connection() {
}

Connection::Connection(const std::string &dsn): Connection() {
    open(dsn) ;
}

void Connection::open(const std::string &dsn) {


    handle_ = DriverFactory::instance().createConnection(dsn) ;

    if ( !handle_ )
        throw Exception("Cannot establish connection with database") ;
}

void Connection::close() {

    handle_->close() ;

}

/*
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
*/
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



} // namespace db
} // namespace wspp
