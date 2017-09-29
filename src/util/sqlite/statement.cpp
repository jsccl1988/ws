#include <wspp/util/sqlite/statement.hpp>
#include <wspp/util/sqlite/connection.hpp>

namespace wspp { namespace util { namespace sql {

Statement::Statement(Connection &con, const std::string & sql) {
    con.check() ;
    stmt_.reset(new Stmt(con.handle(), sql)) ;
}

} // namespace sqlite
} // namespace util
} // namespace wspp
