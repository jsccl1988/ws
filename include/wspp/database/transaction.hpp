#ifndef __DATABASE_TRANSACTION_HPP__
#define __DATABASE_TRANSACTION_HPP__

#include <wspp/database/connection_handle.hpp>

#include <string>

namespace wspp {
namespace db {
class Connection;
class Transaction{
public:
    Transaction(Connection &con_); 

    // you should explicitly call commit or rollback to close it
    void commit();
    void rollback();

private:
    ConnectionHandlePtr con_;
};
} // namespace db
} // namespace wspp

#endif
