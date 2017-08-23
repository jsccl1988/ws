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

#include <wspp/util/detail/value_holder.hpp>

namespace wspp {

class IValueHolder ;

// very lighweight write-only variant class e.g. to enable json responses
//
// e.g.  Variant v(Variant::Object{
//                              {"name", 3},
//                              {"values", Variant::Array{ {2,  string("s") } } }
//                }) ;
//       cout << v.toJSON() << endl ;

class Variant {
public:

    using Object = std::map<std::string, Variant> ;
    using Array = std::vector<Variant> ;

    Variant(): value_(new NullValueHolder()) {}

    template<class T>
    Variant(const T &val): value_(new ValueHolder<T>(val)) {}

    // helper for a key value pair
    Variant(const std::string &key, const std::string &value): Variant(Variant::Object{{key, value}}) {}

    Variant(const Object &values): value_(new ObjectValueHolder(values)) {}
    Variant(const Array &values): value_(new ArrayValueHolder(values)) {}

    bool isObject() const { return value_->isObject() ; }
    bool isArray() const { return value_->isArray() ; }
    bool isNull() const { return value_->isNull() ; }

    Variant at(const std::string &key) const { return value_->fetchKey(key) ; }
    Variant at(uint idx) const { return value_->fetchIndex(idx) ; }


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
