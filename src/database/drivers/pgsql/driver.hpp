#ifndef __DATABASE_PQ_DRIVER_HPP__
#define __DATABASE_PQ_DRIVER_HPP__

#include <memory>
#include <wspp/util/dictionary.hpp>

namespace wspp { namespace db {

class ConnectionHandle ;

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
