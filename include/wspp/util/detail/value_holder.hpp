#ifndef __VALUE_HOLDER_HPP__
#define __VALUE_HOLDER_HPP__

#include <string>
#include <map>
#include <vector>
#include <sstream>

#include <boost/type_traits.hpp>

namespace wspp {
class Variant ;
class IValueHolder {

public:

    virtual void toJSON(std::ostream &) const = 0 ;
    virtual bool isArray() const = 0 ;
    virtual bool isObject() const = 0 ;
    virtual bool isNull() const = 0 ;
    virtual Variant fetchKey(const std::string &key) const = 0 ;
    virtual Variant fetchIndex(uint idx) const = 0 ;

protected:
    static std::string json_escape_string(const std::string &str) ;
};

template <typename T, bool = boost::is_integral<T>::value>
struct JSONWriter {
    static void write(std::ostream &strm, const T& t) {
        strm << t ;
    }
};

template <typename T>
struct JSONWriter<T, true> {
        static void write(std::ostream &strm, const T& t) { strm << t ; }
};

template <typename T>
class ValueHolder: public IValueHolder {
public:
    ValueHolder(const T &val): value_(val) {}

    void toJSON(std::ostream &strm) const override {
        JSONWriter<T>::write(strm, value_) ;
    }

    bool isArray() const override { return false ; }
    bool isObject() const override { return false ; }
    bool isNull() const override { return false ; }
    Variant fetchKey(const std::string &key) const override ;
    Variant fetchIndex(uint idx) const override ;

private:

    T value_ ;
};

class Variant ;
class ObjectValueHolder: public IValueHolder {
public:
    ObjectValueHolder(const std::map<std::string, Variant> &values):
        values_(values) {}

    void toJSON(std::ostream &) const override ;

    bool isArray() const override { return false ; }
    bool isObject() const override { return true ; }
    bool isNull() const override { return false ; }

    Variant fetchKey(const std::string &key) const override ;
    Variant fetchIndex(uint idx) const override ;

    std::map<std::string, Variant> values_ ;
};

class ArrayValueHolder: public IValueHolder {
public:
    ArrayValueHolder(const std::vector<Variant> &values):
        values_(values) {}

    void toJSON(std::ostream &) const override ;
    virtual bool isArray() const override { return true ; }
    virtual bool isObject() const override { return false ; }
    bool isNull() const override { return false ; }

    Variant fetchKey(const std::string &key) const override ;
    Variant fetchIndex(uint idx) const override ;


    std::vector<Variant> values_ ;
};

class NullValueHolder: public IValueHolder {
public:
    NullValueHolder() {}

    void toJSON(std::ostream &) const override ;
    virtual bool isArray() const override { return false ; }
    virtual bool isObject() const override { return false ; }
    bool isNull() const override { return true ; }

    Variant fetchKey(const std::string &key) const override ;
    Variant fetchIndex(uint idx) const override ;
};
}


#endif
