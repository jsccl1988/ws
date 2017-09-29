#include <wspp/util/sqlite/exception.hpp>

using namespace std ;

namespace wspp { namespace util { namespace sql {

Exception::Exception(sqlite3 *handle): std::runtime_error(sqlite3_errmsg(handle)) {}

Exception::Exception(const std::string &msg): std::runtime_error(msg) {}

} // namespace sqlite
} // namespace util
} // namespace wspp
