#ifndef __DATABASE_CONNECTION_HPP__
#define __DATABASE_CONNECTION_HPP__

#include <sqlite3.h>
#include <string>

#include <wspp/database/query.hpp>
#include <wspp/database/connection_handle.hpp>

#include <wspp/util/dictionary.hpp>

namespace wspp { namespace db {

class Statement ;
class Query ;
class Transaction ;


class Connection {

public:

    Connection();
    Connection(const std::string &dsn);
    ~Connection();

    Connection(const Connection &other) = delete ;
    Connection &operator = ( const Connection &other) = delete ;

    // open connection to database withe given params
    void open(const std::string &dsn);
    void close() ;

    operator int () { return (bool)handle_  ; }
    /**
     * @brief Helper for executing an sql statement, including a colon separated list of statements
     * @param sql Format string similar to printf. Use %q for arguments that need quoting (see sqlite3_mprintf documentation)
     */
  //  void exec(const std::string &sqlite, ...) ;

//   sqlite3_int64 last_insert_rowid() {
//       return sqlite3_last_insert_rowid(handle_);
//   }

//    int changes() {
//        return sqlite3_changes(handle_);
//    }

    ConnectionHandlePtr handle() const { return handle_ ; }

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

    std::shared_ptr<ConnectionHandle> handle_ ;
};


} // namespace db
} // namespace wspp


#endif
