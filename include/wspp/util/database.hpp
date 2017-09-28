#ifndef __DATABASE_H__
#define __DATABASE_H__

// Simple C++ API for SQlite database

#include <sqlite3.h>
#include <stdexcept>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <cstdint>
#include <boost/thread/mutex.hpp>
#include <boost/utility.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <wspp/util/dictionary.hpp>

namespace wspp {
namespace util {

namespace sqlite {

class Statement ;
class QueryResult ;
class Exception ;
class NullType;
class Connection ;
class Blob ;

extern NullType Nil;

class Statement ;


class Exception: public std::runtime_error
{
public:
    Exception(const std::string &msg) ;
    Exception(sqlite3 *handle) ;
};

class Binder {
public:

    Binder(Statement &stmt): stmt_(stmt), index_(0) {}

    Statement &stmt_ ;
    uint index_ ;
};

/**
 * @brief The Statement class is a wrapper for prepared statements
 */



class Statement
{
public:

    /**
     * @brief make a new prepared statement from the SQL string and the current connection
     */
    Statement(Connection& con, const std::string & sql) ;

    // helper for creating a connection and binding parameters
    template<typename ...Args>
    Statement(Connection& con, const std::string & sql, Args... args): Statement(con, sql) {
        bindm(args...) ;
    }

    virtual ~Statement();

    Statement( Statement&& other ) {
        std::cout << "ok here" << std::endl ;
    }

    Statement & operator = ( Statement &&other ) {
        std::cout << "ok here" << std::endl ;
    }

    Statement(const Statement &) = delete ;
    Statement & operator = ( const Statement &other ) = delete ;


    /** \brief clear is used if you'd like to reuse a command object
    */
    void clear();

    /** \brief Bind value with corresponding placeholder index
     */

    Statement &bind(int idx, const NullType &) ;
    Statement &bind(int idx, unsigned char v) ;
    Statement &bind(int idx, char v) ;
    Statement &bind(int idx, unsigned short v) ;
    Statement &bind(int idx, short v) ;
    Statement &bind(int idx, unsigned long v) ;
    Statement &bind(int idx, long v) ;
    Statement &bind(int idx, unsigned int v) ;
    Statement &bind(int idx, int v) ;
    Statement &bind(int idx, unsigned long long int v) ;
    Statement &bind(int idx, long long int v) ;
    Statement &bind(int idx, double v) ;
    Statement &bind(int idx, float v) ;
    Statement &bind(int idx, const std::string &v) ;
    Statement &bind(int idx, const Blob &blob) ;

    Statement &bind(int idx, const char *str) ;

    /** \brief Bind value with corresponding placeholder parameter name
     */

    template <class T>
    Statement &bind(const std::string &name, const T &p) {
        int idx = sqlite3_bind_parameter_index(handle_.get(), name.c_str() );
        if ( idx ) return bind(idx, p) ;
        else throw Exception(name + " is not a valid statement placeholder") ;
    }

    /** \brief Bind value with placeholder index automatically assigned based on the order of the calls
     */
    template <class T>
    Statement &bind(const T &p) {
        return bind(++last_arg_idx_, p) ;
    }

    template <typename T>
    Statement &bindm() {
        return *this ;
    }

    template <typename T>
    Statement &bindm(T t) {
        return bind(t) ;
    }

    template <typename First, typename ... Args>
    Statement &bindm(First f, Args ... args) {
        return bind(f).bindm(args...) ;
    }

    sqlite3_stmt *handle() const { return handle_.get() ; }

    void exec() {
        step() ;
    }

    template<typename ...Args>
    Statement &operator()(Args... args) {
        bindm(args...) ;
        exec() ;
        return *this ;
    }

protected:

    void check();
    bool step();

private:

    void prepare();
    void finalize();

    void throwStmtException() ;

protected:

    friend class QueryResult ;

    boost::shared_ptr<sqlite3_stmt> handle_;

private:

    int last_arg_idx_;

};



class Row ;
class Query ;


class Query: public Statement {
public:
    Query(Connection &con, const std::string &sql) ;

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

    QueryResult operator()();

    Query( Query&& other );
    Query & operator = ( Query &&other );

    Query(const Query &) = delete ;
    Query & operator = ( const Query &other ) = delete ;

private:
    friend class QueryResult ;

    int columnIdx(const std::string &name) const ;
    std::map<std::string, int> field_map_ ;
};



class Column {
public:

    template <class T>
    T as() const {
        return qres_.get<T>(idx_) ;
    }

private:

    friend class Row ;

    Column(QueryResult &qr, int idx): qres_(qr), idx_(idx) {}
    Column(QueryResult &qr, const std::string &name);

    QueryResult &qres_ ;
    int idx_ ;
};

class Row {
public:
    Row(QueryResult &qr): qres_(qr) {}

    uint columns() const { return qres_.columns() ; }
    Column operator [] (int idx) const { return Column(qres_, idx); }
    Column operator [] (const std::string &name) const { return Column(qres_, name); }
    bool isValid() const { return (int)qres_ ; }

private:

    QueryResult &qres_ ;
};


// Wraps pointer to buffer and its size. Memory management is external
class Blob {
public:

    Blob(const char *data, uint32_t sz): size_(sz), data_(data) {}

    const char *data() const { return data_ ; }
    uint32_t size() const { return size_ ; }

private:
    const char *data_ = nullptr;
    uint32_t size_ = 0 ;
};

class Connection {

public:

    Connection();
    Connection(const std::string &name, int flags = SQLITE_OPEN_READWRITE);
    ~Connection();

    Connection ( Connection&& other ) ;
    Connection& operator = ( Connection&& other ) ;

    Connection ( const Connection& other ) = delete ;
    Connection& operator = ( const Connection& other ) = delete ;

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

    Statement statement(const std::string &sql) {
        return Statement(*this, sql) ;
    }

    template<typename ...Args>
    Statement statement(Connection& con, const std::string & sql, Args... args) {
        return Statement(*this, args...) ;
    }

    Query query(const std::string &sql) {
        return Query(*this, sql) ;
    }

    template<typename ...Args>
    Query query(Connection& con, const std::string & sql, Args... args) {
        return Query(*this, args...) ;
    }

protected:

    friend class Statement ;
    friend class Transaction ;

    void check() ;

    sqlite3 *handle_ ;
};

class QueryResult
{

public:

    QueryResult(QueryResult &&other) {
        std::cout << "ok here" << std::endl ;
    }
    QueryResult(QueryResult &other) = delete ;
    QueryResult& operator=(const QueryResult &other) = delete;
    QueryResult& operator=(QueryResult &&other) = default;

    bool isValid() const { return !empty_ ; }
    operator int () { return !empty_ ; }

    void next() ;

    int columns() const;
    int columnType(int idx) const;
    const char *columnName(int idx) const ;
    int columnIdx(const std::string &name) const ;
    int columnBytes(int idx) const ;
    bool hasColumn(const std::string &name) const ;

    template<class T>
    T get(int idx) const ;

    template <class T>
    T get(const std::string &name) const {
        int idx = columnIdx(name) ;
        return get<T>(idx) ;
    }

    template<class T>
    void read(int idx, T &val) const ;

    template <typename T>
    void read(T &t) {
        return bind(t) ;
    }

    template <typename ... Args>
    void into(Args &... args) {
        readi(0, args...) ;
    }

    template <typename T>
    void readi(int idx, T &t) {
        read(idx, t) ;
    }

    template <typename First, typename ... Args>
    void readi(int idx, First &f, Args &... args) {
        readi(idx, f) ;
        readi(idx+1, args...) ;
    }

    Dictionary getAll() const ;

    class iterator {
    public:
        iterator(QueryResult &res, bool at_end);
        iterator(const iterator &other) = delete;
        iterator(iterator &&other) = default;

        iterator& operator=(const iterator &other) = delete;
        iterator& operator=(iterator &&other) = default;

        bool operator==(const iterator &other) const { return ( qres_ == other.qres_) && ( at_end_ == other.at_end_ ) ; }
        bool operator!=(const iterator &other) const { return ( qres_ != other.qres_) || ( at_end_ != other.at_end_ ) ; }

        iterator& operator++() {
            qres_.next() ;
            at_end_ = !qres_ ;
            return *this;
        }

        const Row& operator*() const { return *current_; }

    private:
        QueryResult &qres_ ;
        bool at_end_ ;
        std::unique_ptr<Row> current_ ;

    };

    iterator begin() { return iterator(*this, empty_) ; }
    iterator end() { return iterator(*this, true) ; }

private:

    friend class Query ;

    QueryResult(Query &cmd);

    void check() const ;

private:

    Query cmd_ ;
    bool empty_ ;

} ;


class Transaction : boost::noncopyable
{
public:

    Transaction(Connection &con_); // the construcctor starts the constructor

    // you should explicitly call commit or rollback to close it
    void commit();
    void rollback();

private:

    Connection &con_ ;
};




} // namespace sqlite

} // namespace util

} // namespace wspp


#endif
