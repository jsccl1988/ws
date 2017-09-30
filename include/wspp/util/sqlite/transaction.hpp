#ifndef __SQLITE_TRANSACTION_HPP__
#define __SQLITE_TRANSACTION_HPP__

#include <wspp/util/sqlite/connection.hpp>

#include <string>

namespace wspp { namespace util { namespace sqlite {

class Transaction
{
public:

    Transaction(Connection &con_); // the constructor starts the constructor

    // you should explicitly call commit or rollback to close it
    void commit();
    void rollback();

private:

    Connection &con_ ;
};

} // namespace sqlite
} // namespace util
} // namespace wspp


#endif
