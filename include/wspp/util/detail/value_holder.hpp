#ifndef __VALUE_HOLDER_HPP__
#define __VALUE_HOLDER_HPP__

#include <string>
#include <map>
#include <vector>
#include <sstream>

#include <boost/type_traits.hpp>

namespace wspp { namespace util {

class Variant ;
class IValueHolder {

public:
    enum Type { String, Float, Integer, Boolean, Array, Object, Function, Null } ;

    virtual void toJSON(std::ostream &) const = 0 ;
    virtual Type type() const = 0 ;
    virtual bool isFalse() const = 0 ;

    virtual Variant fetchKey(const std::string &key) const = 0 ;
    virtual Variant fetchIndex(uint idx) const = 0 ;
    virtual std::string toString() const = 0 ;

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

    Type type() const override ;

    bool isFalse() const override {
        return false ;
    }

    std::string toString() const override {
        std::ostringstream strm ;
        strm << value_ ;
        return strm.str() ;
    }

    Variant fetchKey(const std::string &key) const override ;
    Variant fetchIndex(uint idx) const override ;

private:

    T value_ ;
};

template<>
inline std::string ValueHolder<std::string>::toString() const { return value_ ; }

template<>
inline IValueHolder::Type ValueHolder<double>::type() const { return IValueHolder::Float ; }

template<>
inline IValueHolder::Type ValueHolder<std::string>::type() const { return IValueHolder::String ; }

template<>
inline IValueHolder::Type ValueHolder<int64_t>::type() const { return IValueHolder::Integer ; }

template<>
inline IValueHolder::Type ValueHolder<bool>::type() const { return IValueHolder::Boolean ; }

template<>
inline bool ValueHolder<bool>::isFalse() const { return !value_ ; }

class Variant ;
class ObjectValueHolder: public IValueHolder {
public:
    ObjectValueHolder(const std::map<std::string, Variant> &values):
        values_(values) {}

    void toJSON(std::ostream &) const override ;

    Type type() const override { return IValueHolder::Object ; }
    std::string toString() const override { return std::string() ; }

    bool isFalse() const override { return false ; }

    Variant fetchKey(const std::string &key) const override ;
    Variant fetchIndex(uint idx) const override ;

    std::map<std::string, Variant> values_ ;
};

class ArrayValueHolder: public IValueHolder {
public:
    ArrayValueHolder(const std::vector<Variant> &values):
        values_(values) {}

    void toJSON(std::ostream &) const override ;

    Type type() const override { return IValueHolder::Array ; }
    bool isFalse() const override { return values_.empty() ; }

    std::string toString() const override { return std::string() ; }

    Variant fetchKey(const std::string &key) const override ;
    Variant fetchIndex(uint idx) const override ;

    std::vector<Variant> values_ ;
};

class NullValueHolder: public IValueHolder {
public:
    NullValueHolder() {}

    void toJSON(std::ostream &) const override ;

    std::string toString() const override { return "null" ; }
    Type type() const override { return IValueHolder::Null ; }
    bool isFalse() const override { return true ; }

    Variant fetchKey(const std::string &key) const override ;
    Variant fetchIndex(uint idx) const override ;
};

} // namespace util
} // namespace wspp

#endif
