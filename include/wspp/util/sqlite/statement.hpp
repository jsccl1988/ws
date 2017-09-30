#ifndef __SQLITE_STATEMENT_HPP__
#define __SQLITE_STATEMENT_HPP__

#include <wspp/util/sqlite/exception.hpp>
#include <wspp/util/sqlite/stmt.hpp>

#include <memory>
#include <sqlite3.h>
#include <string>

namespace wspp { namespace util { namespace sqlite {

class Connection ;

class Statement
{
public:

    Statement(Connection &con, const std::string & sqlite) ;

    // helper for creating a connection and binding parameters
    template<typename ...Args>
    Statement(Connection& con, const std::string & sql, Args... args): Statement(con, sql) {
        stmt_->bindm(args...) ;
    }

    void clear() {
        stmt_->clear() ;
    }

    template <class T>
    Statement &bind(int idx, T v) {
        stmt_->bind(idx, v) ;
        return *this ;
    }

    template <class T>
    Statement &bind(const std::string &name, const T &p) {
        stmt_->bind(name, p) ;
        return *this ;
    }

    template <class T>
    Statement &bind(const T &p) {
        stmt_->bind(p) ;
        return *this ;
    }

    template <typename T>
    Statement &bindm() {
        return *this ;
    }

    template <typename T>
    Statement &bindm(T t) {
        return bind(t) ;
    }

    template <typename ... Args>
    Statement &bindm(Args ... args) {
        stmt_->bindm(args...) ;
        return *this ;
    }

    void exec() {
        stmt_->exec() ;
    }

    template<typename ...Args>
    void operator()(Args... args) {
        bindm(args...) ;
        exec() ;
    }

protected:

    bool step() {
        return stmt_->step() ;
    }

protected:

    std::shared_ptr<Stmt> stmt_ ;

};
} // namespace sqlite
} // namespace util
} // namespace wspp


#endif
