#ifndef __DATABASE_CONNECTION_HANDLE_HPP__
#define __DATABASE_CONNECTION_HANDLE_HPP__

#include <memory>
#include <wspp/database/statement_handle.hpp>

namespace wspp {
namespace db {
class ConnectionHandle {
public:
    ConnectionHandle() = default;

    virtual ~ConnectionHandle() {}
    virtual void close() = 0;
    virtual StatementHandlePtr createStatement(const std::string &sql) = 0;

    virtual void begin() = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;

    virtual uint64_t last_insert_rowid() const = 0;
};

typedef std::shared_ptr<ConnectionHandle> ConnectionHandlePtr;
}
}
#endif
