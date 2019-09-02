#ifndef __DATABASE_EXCEPTION_HPP__
#define __DATABASE_EXCEPTION_HPP__

#include <sqlite3.h>
#include <string>
#include <stdexcept>

namespace wspp {
namespace db {
class ConnectionHandle;
class Exception: public std::runtime_error {
public:
    Exception(const std::string &msg);
    Exception(ConnectionHandle &h);
};
} // namespace sqlite
} // namespace util
#endif
