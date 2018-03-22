#ifndef __DATABASE_PQ_DRIVER_HPP__
#define __DATABASE_PQ_DRIVER_HPP__

#include <memory>
#include <wspp/util/dictionary.hpp>

namespace wspp { namespace db {

class ConnectionHandle ;

// The PostgreSQL driver
//
// The connections string is of the form pgsql:<; delimited option list>
//
// for available options see:
// https://www.postgresql.org/docs/9.4/static/libpq-connect.html

class PQDriver {

public:

    PQDriver() = default ;

    static const PQDriver &instance() {
        static PQDriver instance ;
        return instance ;
    }

    std::shared_ptr<ConnectionHandle> open(const util::Dictionary &params) const ;
};

}
}




#endif
