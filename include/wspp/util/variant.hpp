#ifndef __WSPP_VARIANT_HPP__
#define __WSPP_VARIANT_HPP__

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <functional>
#include <iostream>
#include <iomanip>

#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>

#include <wspp/util/detail/value_holder.hpp>

namespace wspp {

class IValueHolder ;

// very lighweight write-only variant class e.g. to enable json responses and passed to template engines
//
// e.g.  Variant v(Variant::Object{
//                              {"name", 3},
//                              {"values", Variant::Array{ {2,  "s" } } }
//                }) ;
//       cout << v.toJSON() << endl ;

class Variant {
public:

    using Object = std::map<std::string, Variant> ;
    using Array = std::vector<Variant> ;

    // creates a Null value
    Variant(): value_(new NullValueHolder()) {}

    // constructors from simple data types

    Variant(const char *val): value_(new ValueHolder<std::string>(val)) {}
    Variant(const std::string &val): value_(new ValueHolder<std::string>(val)) {}
    Variant(double val): value_(new ValueHolder<double>(val)) {}

    // all integral types are mapped to int64 internaly
    template<class T>
    Variant(T val, typename std::enable_if<std::is_integral<T>::value, T>::type=0 ): value_(new ValueHolder<int64_t>(val)) { }

    Variant(bool val): value_(new ValueHolder<bool>(val)) {}

    // helper for a key value pair
    Variant(const std::string &key, const std::string &value): Variant(Variant::Object{{key, value}}) {}

    Variant(const Object &values): value_(new ObjectValueHolder(values)) {}
    Variant(const Array &values): value_(new ArrayValueHolder(values)) {}

    // check object type
    bool isObject() const { return value_->type() == IValueHolder::Object ; }
    bool isArray() const { return value_->type() == IValueHolder::Array ; }
    bool isNull() const { return value_->type() == IValueHolder::Null ; }
    bool isValue() const {
        return
                ( value_->type() == IValueHolder::String ||
                  value_->type() == IValueHolder::Integer ||
                  value_->type() == IValueHolder::Float ||
                  value_->type() == IValueHolder::Boolean ) ;
    }

    // False are booleans with false values and empty arrays
    bool isFalse() const {
        return value_->isFalse() ;
    }

    bool isFunction() const { return value_->type() == IValueHolder::Function ; }

    // convert value to string
    std::string toString() const {
        if ( isValue() ) {
            return value_->toString() ;
        }
    }

    // length of object or array
    size_t length() const {
        if ( isObject() ) {
            boost::shared_ptr<ObjectValueHolder> e = boost::dynamic_pointer_cast<ObjectValueHolder>(value_) ;
            return e->values_.size() ;
        }
        else if ( isArray() ) {
            boost::shared_ptr<ArrayValueHolder> e = boost::dynamic_pointer_cast<ArrayValueHolder>(value_) ;
            return e->values_.size() ;
        }
        else return 0 ;
    }

    // Returns a member value given the key. The key is of the form <member1>[.<member2>. ... <memberN>]
    // If this is not an object or the key is not found it returns a Null

    Variant at(const std::string &key) const {
        using namespace boost ;
        typedef tokenizer<char_separator<char> > tokenizer;
        char_separator<char> sep(".");

        tokenizer tokens(key, sep);

        boost::shared_ptr<IValueHolder> current = value_ ;
        for ( tokenizer::iterator it = tokens.begin(); it != tokens.end();  ) {
            Variant val = current->fetchKey(*it++) ;
            if ( val.isNull() ) return val ;
            else if ( it != tokens.end() )
                current = val.value_ ;
            else return val ;
        }

        return Variant() ;
    }

    // return an element of an array
    Variant at(uint idx) const { return value_->fetchIndex(idx) ; }

    // convert to JSON string

    std::string toJSON() const {
        std::ostringstream strm ;
        value_->toJSON(strm) ;
        return strm.str() ;
    }

private:

    boost::shared_ptr<IValueHolder> value_ ;
};


}
#include <wspp/util/detail/json.hpp>

#endif
