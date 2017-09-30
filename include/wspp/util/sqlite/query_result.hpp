#ifndef __SQLITE_QUERY_RESULT_HPP__
#define __SQLITE_QUERY_RESULT_HPP__

#include <string>
#include <wspp/util/dictionary.hpp>
#include <wspp/util/sqlite/statement.hpp>

namespace wspp { namespace util { namespace sqlite {

class Row ;
class Query ;
class Column ;

class QueryResult
{

public:

    QueryResult(QueryResult &&other) = default ;
    QueryResult(QueryResult &other) = delete ;
    QueryResult& operator=(const QueryResult &other) = delete;
    QueryResult& operator=(QueryResult &&other) = default;

    // result is not empty
    bool isValid() const { return !empty_ ; }
    operator int () { return !empty_ ; }

    // read next row
    void next() ;

    // number of columns returned
    int columns() const;
    // type of the column
    int columnType(int idx) const;
    // name of the column
    std::string columnName(int idx) const ;
    // index of column with given name
    int columnIdx(const std::string &name) const ;
    // bytes of this column (blobs)
    int columnBytes(int idx) const ;
    // has a column with given name
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

    Column operator [] (const std::string &name);

    Column operator [] (int idx);

    template<class T>
    void read(int idx, T &val) const {
        cmd_->read(idx, val) ;
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

};


class Column {
public:

    template <class T> T as() const {
        return qres_.get<T>(idx_) ;
    }

private:

    friend class Row ;
    friend class QueryResult ;

    Column(QueryResult &qr, int idx): qres_(qr), idx_(idx) {}
    Column(QueryResult &qr, const std::string &name): qres_(qr), idx_(qr.columnIdx(name)) {}

    QueryResult &qres_ ;
    int idx_ ;
};




inline Column QueryResult::operator [](const std::string &name) {
    return Column(*this, name) ;
}

inline Column QueryResult::operator [](int idx) {
    return Column(*this, idx) ;
}

class ColumnAccessor ;

class Row {
public:
    Row(QueryResult &qr): qres_(qr) {}


    Column operator [] (int idx) const { return Column(qres_, idx); }
    Column operator [] (const std::string &name) const { return Column(qres_, name); }
    bool isValid() const { return (int)qres_ ; }

    // number of columns returned
    int columns() const { return qres_.columns(); }
    // type of the column
    int columnType(int idx) const { return qres_.columnType(idx); }
    // name of the column
    std::string columnName(int idx) const { return qres_.columnName(idx); }
    // index of column with given name
    int columnIdx(const std::string &name) const { return qres_.columnIdx(name) ; }
    // bytes of this column (blobs)
    int columnBytes(int idx) const { return qres_.columnBytes(idx); }
    // has a column with given name
    bool hasColumn(const std::string &name) const { return qres_.hasColumn(name); }

    template <typename ... Args>
    void into(Args &... args) {
        qres_.into(args...) ;
    }

    template<class T>
    void read(int idx, T &val) const {
        qres_.read(idx, val) ;
    }

    template<class T>
    T get(int idx) const {
        return qres_.get<T>(idx) ;
    }

    template <class T>
    T get(const std::string &name) const {
        return qres_.get<T>(name) ;
    }

    template<class T>
    friend ColumnAccessor operator >> ( const Row &row, T &val ) ;

private:

    QueryResult &qres_ ;
};

class ColumnAccessor {
public:

    ColumnAccessor(const Row &row): row_(row), idx_(0) {}

    template<class T>
    ColumnAccessor &operator >> (T &v) {
        row_.read(++idx_, v) ;
        return *this ;
    }

    const Row &row_ ;
    int idx_ ;
};


template<class T>
ColumnAccessor operator >> ( const Row &row, T &val ) {
    row.read(0, val) ;
    return ColumnAccessor(row) ;
}

} // namespace sqlite
} // namespace util
} // namespace wspp


#endif
