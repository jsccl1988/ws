#ifndef __SQLITE_QUERY_RESULT_HPP__
#define __SQLITE_QUERY_RESULT_HPP__

#include <string>
#include <wspp/util/dictionary.hpp>
#include <wspp/util/sqlite/statement.hpp>

namespace wspp { namespace util { namespace sql {

class Row ;
class Query ;

class QueryResult
{

public:

    QueryResult(QueryResult &&other) = default ;
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
    T get(int idx) const {
        return cmd_->get<T>(idx) ;
    }

    template <class T>
    T get(const std::string &name) const {
        int idx = columnIdx(name) ;
        return get<T>(idx) ;
    }

    template<class T>
    void read(int idx, T &val) const {
        cmd_->read(idx, val) ;
    }

    template <typename T>
    void read(T &t) {
        return cmd_->bind(t) ;
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

    QueryResult(std::shared_ptr<Stmt> cmd);

private:

    std::shared_ptr<Stmt> cmd_ ;
    bool empty_ ;

} ;


class Column {
public:

    template <class T> T as() const {
        return qres_.get<T>(idx_) ;
    }

private:

    friend class Row ;

    Column(QueryResult &qr, int idx): qres_(qr), idx_(idx) {}
    Column(QueryResult &qr, const std::string &name): qres_(qr), idx_(qr.columnIdx(name)) {}

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


} // namespace sqlite
} // namespace util
} // namespace wspp


#endif
