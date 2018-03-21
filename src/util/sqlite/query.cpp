#include <wspp/util/sqlite/query.hpp>
#include <wspp/util/sqlite/connection.hpp>

using namespace std ;
namespace wspp { namespace util { namespace sqlite {

Query::Query(Connection &con, const string &sql):
    Statement(con, sql) {
}


QueryResult Query::exec() {
    return QueryResult(stmt_) ;
}

} // namespace sqlite
} // namespace util
} // namespace wspp
