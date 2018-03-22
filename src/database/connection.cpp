#include <wspp/database/connection.hpp>
#include <wspp/database/exception.hpp>
#include <wspp/database/transaction.hpp>

#include <wspp/database/connection_handle.hpp>

#include "driver_factory.hpp"

#include <fstream>
#include <boost/algorithm/string.hpp>

using namespace std ;

namespace wspp { namespace db {

Connection::Connection() {
}

Connection::Connection(const std::string &dsn): Connection() {
    open(dsn) ;
}

void Connection::open(const std::string &dsn) {

    if ( boost::starts_with(dsn, "uri:file://") ) {
        string fdsn ;
        ifstream strm(dsn.substr(11)) ;
        std::getline(strm, fdsn) ;
        handle_ = DriverFactory::instance().createConnection(fdsn) ;
    }
    else
        handle_ = DriverFactory::instance().createConnection(dsn) ;

    if ( !handle_ )
        throw Exception("Cannot establish connection with database") ;
}

void Connection::close() {

    handle_->close() ;

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




} // namespace db
} // namespace wspp
