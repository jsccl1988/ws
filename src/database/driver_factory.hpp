#ifndef __DATABASE_DRIVER_FACTORY_HPP__
#define __DATABASE_DRIVER_FACTORY_HPP__

#include <string>

#include <wspp/database/connection.hpp>
#include <wspp/util/dictionary.hpp>

namespace wspp { namespace db {

class DriverFactory {

public:

    DriverFactory() = default ;

    // open connection to database with given params
    std::shared_ptr<ConnectionHandle> createConnection(const std::string &dsn) const ;

    static const DriverFactory &instance() {
        static DriverFactory factory ;
        return factory ;
    }

    static bool parseParamString(const std::string &str, util::Dictionary &params) ;

};


} // namespace db
} // namespace wspp


#endif
