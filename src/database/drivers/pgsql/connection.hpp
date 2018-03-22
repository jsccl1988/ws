#ifndef __DATABASE_PQ_DRIVER_CONNECTION_HPP__
#define __DATABASE_PQ_DRIVER_CONNECTION_HPP__

#include <libpq-fe.h>

#include <wspp/database/connection_handle.hpp>

namespace wspp { namespace db {

class PQConnectionHandle: public ConnectionHandle {
public:
    PQConnectionHandle(PGconn *handle): handle_(handle) {}
    ~PQConnectionHandle() { close() ; }

    void close() override ;

    StatementHandlePtr createStatement(const std::string &sql) ;

    void begin() override ;
    void commit() override ;
    void rollback() override ;

    uint64_t last_insert_rowid() const override ;

private:

    void exec(const char *sql);

    PGconn *handle_ ;
};



} // namespace db
} // namespace wspp

#endif
