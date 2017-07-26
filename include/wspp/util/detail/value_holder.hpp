#ifndef __VALUE_HOLDER_HPP__
#define __VALUE_HOLDER_HPP__

#include <string>
#include <map>
#include <vector>
#include <sstream>

#include <boost/type_traits.hpp>

namespace wspp {

class IValueHolder {

public:

    virtual void toJSON(std::ostream &) const = 0 ;
//    virtual std::string toString() const = 0 ;
//    virtual bool fromString(const std::string &val) = 0 ;

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

private:

    T value_ ;
};

class Variant ;
class ObjectValueHolder: public IValueHolder {
public:
    ObjectValueHolder(const std::map<std::string, Variant> &values):
        values_(values) {}

    void toJSON(std::ostream &) const override ;

    std::map<std::string, Variant> values_ ;
};

class ArrayValueHolder: public IValueHolder {
public:
    ArrayValueHolder(const std::vector<Variant> &values):
        values_(values) {}

    void toJSON(std::ostream &) const override ;

    std::vector<Variant> values_ ;
};


}


#endif
