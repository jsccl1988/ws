#ifndef __DATABASE_STATEMENT_HANDLE_HPP__
#define __DATABASE_STATEMENT_HANDLE_HPP__

#include <wspp/database/exception.hpp>
#include <wspp/database/query_result.hpp>
#include <wspp/database/types.hpp>

#include <boost/optional.hpp>

#include <string>
#include <map>
#include <memory>

namespace wspp {
namespace db {
class ConnectionHandle;
class StatementHandle{
public:
    virtual ~StatementHandle() {}

    virtual void clear() = 0;
    virtual void finalize() = 0;

    virtual StatementHandle &bind(int idx, const NullType &) = 0;
    virtual StatementHandle &bind(int idx, unsigned char v) = 0;
    virtual StatementHandle &bind(int idx, char v) = 0;
    virtual StatementHandle &bind(int idx, unsigned short v) = 0;
    virtual StatementHandle &bind(int idx, short v) = 0;
    virtual StatementHandle &bind(int idx, unsigned long v) = 0;
    virtual StatementHandle &bind(int idx, long v) = 0;
    virtual StatementHandle &bind(int idx, unsigned int v) = 0;
    virtual StatementHandle &bind(int idx, int v) = 0;
    virtual StatementHandle &bind(int idx, unsigned long long int v) = 0;
    virtual StatementHandle &bind(int idx, long long int v) = 0;
    virtual StatementHandle &bind(int idx, double v) = 0;
    virtual StatementHandle &bind(int idx, float v) = 0;
    virtual StatementHandle &bind(int idx, const std::string &v) = 0;
    virtual StatementHandle &bind(int idx, const Blob &blob) = 0;
    virtual StatementHandle &bind(int idx, const char *str) = 0;

    // note that not all drivers support this
    virtual int placeholderNameToIndex(const std::string &name) = 0;

    virtual void exec() = 0;
    virtual QueryResult execQuery() = 0;
};

typedef std::shared_ptr<StatementHandle> StatementHandlePtr;
} // namespace util
} // namespace wspp
#endif
