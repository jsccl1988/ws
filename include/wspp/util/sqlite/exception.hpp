#ifndef __SQLITE_EXCEPTION_HPP__
#define __SQLITE_EXCEPTION_HPP__

#include <sqlite3.h>
#include <string>
#include <stdexcept>

namespace wspp { namespace util { namespace sqlite {

class Exception: public std::runtime_error
{
public:
    Exception(const std::string &msg) ;
    Exception(sqlite3 *handle) ;
};

} // namespace sqlite
} // namespace util
} // namespace wspp


#endif
