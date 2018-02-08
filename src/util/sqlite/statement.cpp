#include <wspp/util/sqlite/statement.hpp>
#include <wspp/util/sqlite/connection.hpp>

#include <boost/algorithm/string.hpp>

using namespace std ;

namespace wspp { namespace util { namespace sqlite {

Statement::Statement(Connection &con, const std::string & sql) {
    con.check() ;
    stmt_.reset(new Stmt(con.handle(), sql)) ;
}

std::string escapeName(const std::string &unescaped) {
    string e = boost::algorithm::replace_all_copy(unescaped, "\"", "\"\"") ;
    return '"' + e + '"' ;
}

} // namespace sqlite
} // namespace util
} // namespace wspp
