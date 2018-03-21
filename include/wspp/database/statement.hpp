#ifndef __DATABASE_STATEMENT_HPP__
#define __DATABASE_STATEMENT_HPP__

#include <wspp/database/exception.hpp>
#include <wspp/database/statement_handle.hpp>

#include <memory>
#include <string>

namespace wspp { namespace db {

class Connection ;

// escape column names/identifiers

std::string escapeName(const std::string &unescaped) ;

class Statement
{
public:

    Statement(Connection &con, const std::string &sql) ;

    // helper for creating a connection and binding parameters
    template<typename ...Args>
    Statement(Connection& con, const std::string & sql, Args... args): Statement(con, sql) {
        bindAll(args...) ;
    }

    // clear parameter bindings

    void clear() {
        stmt_->clear() ;
    }

    // bind value to placeholder index

    template <class T>
    Statement &bind(int idx, T v) {
        stmt_->bind(idx, v) ;
        return *this ;
    }

    // bind value by to placeholder by name

    template <class T>
    Statement &bind(const std::string &name, const T &p) {
        stmt_->bind(stmt_->placeholderNameToIndex(name), p) ;
        return *this ;
    }

    // bind all values sequentially

    template <typename ... Args>
    Statement &bindAll(Args ... args) {
        return bindm(1, args...) ;
    }

    // bind values and execute statement

    template<typename ...Args>
    void operator()(Args... args) {
        bindAll(args...) ;
        exec() ;
    }

    void exec() {
        stmt_->exec() ;
    }

protected:

    template <typename T>
    Statement &bindm(uint idx) {
        return *this ;
    }

    template <typename T>
    Statement &bindm(uint idx, T t) {
        return bind(idx, t) ;
    }

    template <typename First, typename ... Args>
    Statement &bindm(uint idx, First f, Args ... args) {
        return bind(idx++, f).bindm(idx, args...) ;
    }

protected:

    StatementHandlePtr stmt_ ;
};

} // namespace db
} // namespace wspp


#endif
