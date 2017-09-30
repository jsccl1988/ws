#ifndef __SQLITE_QUERY_HPP__
#define __SQLITE_QUERY_HPP__

#include <wspp/util/sqlite/statement.hpp>
#include <wspp/util/sqlite/query_result.hpp>
#include <string>

namespace wspp { namespace util { namespace sqlite {

class Connection ;
class QueryResult ;

class Query: public Statement {
public:
    Query(Connection &con, const std::string &sqlite) ;

    template<typename ...Args>
    Query(Connection& con, const std::string & sql, Args... args): Query(con, sql) {
        bindm(args...) ;
    }

    QueryResult exec() ;

    template<typename ...Args>
    QueryResult operator()(Args... args) {
        bindm(args...) ;
        return exec() ;
    }

    QueryResult operator()() {
        return exec() ;
    }
};

} // namespace sqlite
} // namespace util
} // namespace wspp


#endif
