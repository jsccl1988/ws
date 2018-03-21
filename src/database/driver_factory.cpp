#include "driver_factory.hpp"

#include "drivers/sqlite/driver.hpp"

using namespace std ;

namespace wspp {
namespace db {

std::shared_ptr<ConnectionHandle> DriverFactory::createConnection(const std::string &dsn) const
{
    int pos = dsn.find_first_of(':') ;
    if ( pos == string::npos ) return nullptr ;

    string driver_name = dsn.substr(0, pos) ;
    string params = dsn.substr(pos+1) ;

    if ( driver_name == "sqlite" ) {
        return SQLiteDriver::instance().open(params) ;
    }

    return nullptr ;

}

}
}
