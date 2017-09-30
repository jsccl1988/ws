#ifndef __SQLITE_STMT_HPP__
#define __SQLITE_STMT_HPP__

#include <wspp/util/sqlite/exception.hpp>
#include <wspp/util/sqlite/types.hpp>

#include <sqlite3.h>
#include <string>
#include <map>

namespace wspp { namespace util { namespace sqlite {

// wrapper for a sqlite3_stmt

class Stmt
{
public:

    Stmt(sqlite3 *con_handle, const std::string & sqlite) ;
    ~Stmt();

    void clear();

    Stmt &bind(int idx, const NullType &) ;
    Stmt &bind(int idx, unsigned char v) ;
    Stmt &bind(int idx, char v) ;
    Stmt &bind(int idx, unsigned short v) ;
    Stmt &bind(int idx, short v) ;
    Stmt &bind(int idx, unsigned long v) ;
    Stmt &bind(int idx, long v) ;
    Stmt &bind(int idx, unsigned int v) ;
    Stmt &bind(int idx, int v) ;
    Stmt &bind(int idx, unsigned long long int v) ;
    Stmt &bind(int idx, long long int v) ;
    Stmt &bind(int idx, double v) ;
    Stmt &bind(int idx, float v) ;
    Stmt &bind(int idx, const std::string &v) ;
    Stmt &bind(int idx, const Blob &blob) ;

    Stmt &bind(int idx, const char *str) ;

    template <class T>
    Stmt &bind(const std::string &name, const T &p) {
        int idx = sqlite3_bind_parameter_index(handle_, name.c_str() );
        if ( idx ) return bind(idx, p) ;
        else throw Exception(name + " is not a valid statement placeholder") ;
    }

    template <class T>
    Stmt &bind(const T &p) {
        return bind(++last_arg_idx_, p) ;
    }

    template <typename T>
    Stmt &bindm() {
        return *this ;
    }

    template <typename T>
    Stmt &bindm(T t) {
        return bind(t) ;
    }

    template <typename First, typename ... Args>
    Stmt &bindm(First f, Args ... args) {
        return bind(f).bindm(args...) ;
    }

    sqlite3_stmt *handle() const { return handle_ ; }

    void exec() {
        step() ;
    }

    template<typename ...Args>
    void operator()(Args... args) {
        bindm(args...) ;
        exec() ;
    }

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

    void check() const ;
    bool step();

    int columnIdx(const std::string &name) const ;
    int columns() const;
    const char *columnName(int idx) const;
    int columnType(int idx) const;
    int columnBytes(int idx) const;

private:

    void prepare();
    void finalize();

    void throwStmtException() ;

protected:

    sqlite3_stmt *handle_;
    int last_arg_idx_;
    std::map<std::string, int> field_map_ ;
};

} // namespace sqlite
} // namespace util
} // namespace wspp


#endif
