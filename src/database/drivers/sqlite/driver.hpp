#ifndef __DATABASE_SQLITE_DRIVER_HPP__
#define __DATABASE_SQLITE_DRIVER_HPP__

#include <memory>

namespace wspp { namespace db {

class ConnectionHandle ;

class SQLiteDriver {

public:

    SQLiteDriver() = default ;

    static const SQLiteDriver &instance() {
        static SQLiteDriver instance ;
        return instance ;
    }

    std::shared_ptr<ConnectionHandle> open(const std::string &params) const ;
};

}
}




#endif
