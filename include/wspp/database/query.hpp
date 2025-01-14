#ifndef __DATABASE_QUERY_HPP__
#define __DATABASE_QUERY_HPP__

#include <wspp/database/statement.hpp>
#include <wspp/database/query_result.hpp>
#include <string>

namespace wspp {
namespace db {
class Connection;
class QueryResult;
class Query: public Statement {
public:
    Query(Connection &con, const std::string &sqlite);

    template<typename ...Args>
    Query(Connection& con, const std::string & sql, Args... args): Query(con, sql) {
        bindAll(args...);
    }

    QueryResult exec();

    template<typename ...Args>
    QueryResult operator()(Args... args) {
        bindAll(args...);
        return exec();
    }

    QueryResult operator()() {
        return exec();
    }
};
} // namespace db
} // namespace wspp
#endif
