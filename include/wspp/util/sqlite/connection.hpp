#ifndef __SQLITE_CONNECTION_HPP__
#define __SQLITE_CONNECTION_HPP__

#include <sqlite3.h>
#include <string>

#include <wspp/util/sqlite/query.hpp>

namespace wspp { namespace util { namespace sql {

class Statement ;
class Query ;
class Transaction ;

class Connection {

public:

    Connection();
    Connection(const std::string &name, int flags = SQLITE_OPEN_READWRITE);
    ~Connection();

    Connection(const Connection &other) = delete ;
    Connection &operator = ( const Connection &other) = delete ;

    // open connection to database withe given flags
    void open(const std::string &name, int flags = SQLITE_OPEN_READWRITE);
    void close() ;

    operator int () { return handle_ != nullptr ; }
    /**
     * @brief Helper for executing an sql statement, including a colon separated list of statements
     * @param sql Format string similar to printf. Use %q for arguments that need quoting (see sqlite3_mprintf documentation)
     */
    void exec(const std::string &sql, ...) ;

    sqlite3_int64 last_insert_rowid() {
        return sqlite3_last_insert_rowid(handle_);
    }

    int changes() {
        return sqlite3_changes(handle_);
    }

    sqlite3 *handle() { return handle_ ; }

    Statement prepareStatement(const std::string &sql) ;
    Query prepareQuery(const std::string &sql) ;

    // helper for creating a connection and binding parameters
    template<typename ...Args>
    QueryResult query(const std::string & sql, Args... args) {
        return Query(*this, sql)(args...) ;
    }

    template<typename ...Args>
    void execute(const std::string &sql, Args... args) {
        Statement(*this, sql)(args...) ;
    }

    Transaction transaction() ;

    void check() ;

protected:

    sqlite3 *handle_ ;
};



} // namespace sqlite
} // namespace util
} // namespace wspp


#endif
