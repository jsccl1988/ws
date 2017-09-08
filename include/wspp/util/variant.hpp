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

#include <wspp/util/dictionary.hpp>
#include <wspp/util/detail/value_holder.hpp>

class JSONParser ;

namespace wspp { namespace util {

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

    static Variant fromDictionary(const Dictionary &dict) {
        Variant::Object obj ;
        for( const auto &p: dict )
            obj.insert({p.first, p.second}) ;
        return obj ;
    }

    static Variant fromDictionaryAsArray(const Dictionary &dict, const std::string &keyname = "key", const std::string &valname = "val") {
        Variant::Array ar ;
        for( const auto &p: dict )
            ar.emplace_back(Variant::Object({{keyname, p.first}, {valname, p.second}})) ;
        return ar ;
    }

    template<class T>
    static Variant fromVector(const std::vector<T> &vals) {
        Variant::Array ar ;
        for( const auto &p: vals )
            ar.push_back(p) ;
        return ar ;
    }

    // may optionaly throw a JSONParseException ;
    static Variant fromJSONString(const std::string &src, bool throw_exception = false) ;
    static Variant fromJSONFile(const std::string &path, bool throw_exception = false) ;

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

    std::vector<std::string> keys() const {
        std::vector<std::string> res ;

        if ( !isObject() ) return res ;

         boost::shared_ptr<ObjectValueHolder> e = boost::dynamic_pointer_cast<ObjectValueHolder>(value_) ;

         for( const auto &p: e->values_ )
             res.push_back(p.first) ;

         return res ;
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

class JSONParseException: public std::exception {
private:
    friend class ::JSONParser ;
    JSONParseException(const std::string &msg, uint line, uint col) ;
public:
    const char *what() const noexcept override {
        return msg_.c_str() ;
    }
protected:
    std::string msg_ ;
};


template <class T>
inline Variant ValueHolder<T>::fetchKey(const std::string &key) const {
    return Variant() ;
}

template <class T>
inline Variant ValueHolder<T>::fetchIndex(uint idx) const {
    return Variant() ;
}

inline Variant ArrayValueHolder::fetchKey(const std::string &key) const {
    return Variant() ;
}

inline Variant ArrayValueHolder::fetchIndex(uint idx) const {
    if ( idx < values_.size() )
        return values_[idx] ;
    else
    return Variant() ;
}

inline Variant ObjectValueHolder::fetchKey(const std::string &key) const {
    auto it = values_.find(key) ;
    if ( it == values_.end() )
        return Variant() ;
    else return it->second ;
}

inline Variant ObjectValueHolder::fetchIndex(uint idx) const {
    return Variant() ;
}

inline Variant NullValueHolder::fetchKey(const std::string &key) const {
    return Variant() ;
}

inline Variant NullValueHolder::fetchIndex(uint idx) const {
    return Variant() ;
}


template<>
inline void ValueHolder<std::string>::toJSON(std::ostream &strm) const {
    strm << json_escape_string(value_) ;
}

template<>
inline void ValueHolder<bool>::toJSON(std::ostream &strm) const {
    strm << ( value_ ? "true" : "false") ;
}

inline void NullValueHolder::toJSON(std::ostream &strm) const {
    strm << "null" ;
}


inline void ObjectValueHolder::toJSON(std::ostream &strm) const {


    strm << "{" ;
    auto it = values_.cbegin() ;
    if ( it != values_.cend() ) {
        strm << json_escape_string(it->first) << ": " << it->second.toJSON() ;
        ++it ;
    }
    while ( it != values_.cend() ) {
        strm << ", " ;
        strm << json_escape_string(it->first) << ": " << it->second.toJSON() ;
        ++it ;
    }

    strm << "}" ;
}


inline void ArrayValueHolder::toJSON(std::ostream &strm) const {

    strm << "[" ;
    auto it = values_.cbegin() ;
    if ( it != values_.cend() ) {
        strm << it->toJSON() ;
        ++it ;
    }
    while ( it != values_.cend() ) {
        strm << ", " ;
        strm << it->toJSON() ;
        ++it ;
    }

    strm << "]" ;
}

// Original: https://gist.github.com/kevinkreiser/bee394c60c615e0acdad

inline std::string IValueHolder::json_escape_string(const std::string &str) {
    std::stringstream strm ;
    strm << '"';

    for (const auto& c : str) {
        switch (c) {
        case '\\': strm << "\\\\"; break;
        case '"': strm << "\\\""; break;
        case '/': strm << "\\/"; break;
        case '\b': strm << "\\b"; break;
        case '\f': strm << "\\f"; break;
        case '\n': strm << "\\n"; break;
        case '\r': strm << "\\r"; break;
        case '\t': strm << "\\t"; break;
        default:
            if(c >= 0 && c < 32) {
                //format changes for json hex
                strm.setf(std::ios::hex, std::ios::basefield);
                strm.setf(std::ios::uppercase);
                strm.fill('0');
                //output hex
                strm << "\\u" << std::setw(4) << static_cast<int>(c);
            }
            else
                strm << c;
            break;
        }
    }
    strm << '"';

    return strm.str() ;
}

} // namespace util

} // namespace wspp

#endif
