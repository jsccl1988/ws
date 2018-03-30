#ifndef __WSPP_UTIL_VARIANT_HPP__
#define __WSPP_UTIL_VARIANT_HPP__

#include <cstdint>
#include <map>
#include <vector>
#include <iomanip>
#include <iostream>

#include <wspp/util/dictionary.hpp>

#include <boost/tokenizer.hpp>

// very lighweight write-only variant class e.g. to enable json responses and passed to template engines
//
// e.g.  Variant v(Variant::Object{
//                              {"name", 3},
//                              {"values", Variant::Array{ {2,  "s" } } }
//                }) ;
//       cout << v.toJSON() << endl ;

namespace wspp {
    namespace twig {
        class TemplateEvalContext ;
    }
}


namespace wspp { namespace util {

class Variant {

public:

    using Object = std::map<std::string, Variant> ;
    using Array = std::vector<Variant> ;
    using Function = std::function<Variant(const Variant &, twig::TemplateEvalContext &)> ;

    using signed_integer_t = int64_t ;
    using unsigned_integer_t = uint64_t ;
    using float_t = double ;
    using string_t = std::string ;
    using boolean_t = bool ;


    enum class Type : uint8_t {
        Undefined, Null,  Object, Array, String, Boolean, SInteger, UInteger, Float, Function
    };
    // constructors

    Variant(): tag_(Type::Null) {}

    Variant(boolean_t v) noexcept : tag_(Type::Boolean) { data_.b_ = v ; }

    Variant(int v) noexcept: Variant((int64_t)v) {}
    Variant(unsigned int v) noexcept: Variant((uint64_t)v) {}

    Variant(int64_t v) noexcept {
        if ( v < 0 ) {
            tag_ = Type::SInteger ;
            data_.si_ = v ;
        } else {
            tag_ = Type::UInteger ;
            data_.ui_ = v ;
        }
    }

    Variant(uint64_t v) noexcept: tag_(Type::UInteger) { data_.ui_ = v ; }

    Variant(Function v): tag_(Type::Function) { new (&data_.fp_) Function(v) ; }

    Variant(float_t v) noexcept: tag_(Type::Float) { data_.f_ = v ; }

    Variant(const char *value): tag_(Type::String) {
        new (&data_.s_) std::string(value) ;
    }

    Variant(const string_t& value): tag_(Type::String) {
        new (&data_.s_) std::string(value) ;
    }

    Variant(string_t&& value): tag_(Type::String)  {
        new (&data_.s_) std::string(std::move(value)) ;
    }

    Variant(const Object& value): tag_(Type::Object) {
        new (&data_.o_) Object(value) ;
    }

    Variant(Object&& value): tag_(Type::Object) {
        new (&data_.o_) Object(std::move(value)) ;
    }

    Variant(const Array& value): tag_(Type::Array) {
        new (&data_.a_) Array(value) ;
    }

    Variant(Array&& value): tag_(Type::Array) {
        new (&data_.a_) Array(std::move(value)) ;
    }

    ~Variant() {
        destroy() ;
    }

    Variant(const Variant& other) {
        create(other) ;
    }

    Variant &operator=(const Variant &other) {
        if ( this != &other ) {
            destroy() ;
            create(other) ;
        }
        return *this ;
    }

    Variant(Variant&& other): tag_(other.tag_) {
        switch (tag_)
        {
        case Type::Object:
            new (&data_.o_) Object(std::move(other.data_.o_)) ;
            break;
        case Type::Array:
            new (&data_.a_) Array(std::move(other.data_.a_)) ;
            break;
        case Type::String:
            new (&data_.s_) std::string(std::move(other.data_.s_)) ;
            break;
        case Type::Boolean:
            data_.b_ = other.data_.b_ ;
            break;
        case Type::SInteger:
            data_.si_ = other.data_.si_ ;
            break;
        case Type::UInteger:
            data_.ui_ = other.data_.ui_ ;
            break;
        case Type::Float:
            data_.f_ = other.data_.f_ ;
            break;
        case Type::Function:
            new (&data_.fp_) Function(std::move(other.data_.fp_)) ;
        default:
            break;
        }

        other.tag_ = Type::Undefined ;
    }

    // make Object from a dictionary
    static Variant fromDictionary(const Dictionary &dict) {
        Variant::Object obj ;
        for( const auto &p: dict )
            obj.insert({p.first, p.second}) ;
        return obj ;
    }

    // make Array from dictionary where each element is the Object {<keyname>: <key>, <valname>: <val>}
    static Variant fromDictionaryAsArray(const Dictionary &dict, const std::string &keyname = "key", const std::string &valname = "val") {
        Variant::Array ar ;
        for( const auto &p: dict )
            ar.emplace_back(Variant::Object({{keyname, p.first}, {valname, p.second}})) ;
        return ar ;
    }

    // make Array from a vector of values
    template<class T>
    static Variant fromVector(const std::vector<T> &vals) {
        Variant::Array ar ;
        for( const auto &p: vals )
            ar.push_back(p) ;
        return ar ;
    }

    // Parse JSON string into Variant. May optionaly throw a JSONParseException or otherwise return a Null ;
    static Variant fromJSONString(const std::string &src, bool throw_exception = false) ;
    static Variant fromJSONFile(const std::string &path, bool throw_exception = false) ;

    // check object type

    bool isObject() const { return tag_ == Type::Object ; }
    bool isArray() const { return tag_ == Type::Array ; }
    bool isNull() const { return tag_ == Type::Null ; }
    bool isUndefined() const { return tag_ == Type::Undefined ; }

    bool isString() const { return tag_ == Type::String ; }
    bool isNumber() const {
        return ( tag_ == Type::SInteger ) ||
                ( tag_ == Type::UInteger ) ||
                ( tag_ == Type::Float ) ;
    }

    // check if variant stores simple type string, number, integer or boolean
    bool isPrimitive() const {
        return ( tag_ == Type::String ||
                 tag_ == Type::SInteger ||
                 tag_ == Type::UInteger ||
                 tag_ == Type::Float ||
                 tag_ == Type::Boolean
                 ) ;
    }

    // False are booleans with false values and empty arrays
    bool isFalse() const {
        if ( tag_ == Type::Boolean ) return !data_.b_ ;
        else if ( tag_ == Type::Array ) return data_.a_.size() == 0 ;
        else if ( tag_ == Type::Null ) return true ;
        else return false ;
    }

    // convert value to string
    std::string toString() const {
        switch (tag_)
        {
        case Type::String:
            return data_.s_;
        case Type::Boolean: {
            std::ostringstream strm ;
            strm << data_.b_ ;
            return strm.str() ;
        }
        case Type::SInteger:
            return std::to_string(data_.si_) ;
        case Type::UInteger:
            return std::to_string(data_.ui_) ;
        case Type::Float:
            return std::to_string(data_.f_) ;
        default:
            return std::string();
        }
    }

    double toNumber() const {
        switch (tag_)
        {
        case Type::String:
            try {
            return std::stod(data_.s_);
        }
            catch ( ... ) {
            return 0.0 ;
        }

        case Type::Boolean:
            return (double)data_.b_ ;
        case Type::SInteger:
            return (double)data_.si_ ;
        case Type::UInteger:
            return (double)data_.ui_ ;
        case Type::Float:
            return (double)data_.f_ ;
        default:
            return 0.0;
        }
    }

    bool toBoolean() const {
        switch (tag_)
        {
        case Type::String:
            return !(data_.s_.empty()) ;
        case Type::Boolean:
            return data_.b_ ;
        case Type::SInteger:
            return (bool)data_.si_ ;
        case Type::UInteger:
            return (bool)data_.ui_ ;
        case Type::Float:
            return data_.f_ != 0 ;
        default:
            return false;
        }

    }

    // Return the keys of an Object otherwise an empty list
    std::vector<std::string> keys() const {
        std::vector<std::string> res ;

        if ( !isObject() ) return res ;

        for ( const auto &p: data_.o_ )
            res.push_back(p.first) ;
        return res ;
    }

    // length of object or array, zero otherwise
    size_t length() const {
        if ( isObject() )
            return data_.o_.size() ;
        else if ( isArray() ) {
            return data_.a_.size() ;
        }
        else return 0 ;
    }

    // Returns a member value given the key. The key is of the form <member1>[.<member2>. ... <memberN>]
    // If this is not an object or the key is not found it returns a Null

    const Variant &at(const std::string &key) const {
        using namespace boost ;
        typedef tokenizer<char_separator<char> > tokenizer;
        char_separator<char> sep(".");

        tokenizer tokens(key, sep);

        const Variant *current = this ;
        if ( !current->isObject() ) return undefined() ;

        for ( tokenizer::iterator it = tokens.begin(); it != tokens.end();  ) {
            const Variant &val = current->fetchKey(*it++) ;
            if ( val.isUndefined() ) return val ;
            else if ( it != tokens.end() )
                current = &val ;
            else return val ;
        }

        return Variant::undefined() ;
    }

    // return an element of an array
    const Variant &at(uint idx) const { return fetchIndex(idx) ; }

    // overloaded indexing operators
    const Variant &operator [] (const std::string &key) const {
        return fetchKey(key) ;
    }

    const Variant &operator [] (uint idx) const {
        return fetchIndex(idx) ;
    }

    Type type() const { return tag_ ; }

    // JSON encoder
    void toJSON(std::ostream &strm) const {

        switch ( tag_ ) {
        case Type::Object: {
            strm << "{" ;
            auto it = data_.o_.cbegin() ;
            if ( it != data_.o_.cend() ) {
                strm << json_escape_string(it->first) << ": " ;
                it->second.toJSON(strm) ;
                ++it ;
            }
            while ( it != data_.o_.cend() ) {
                strm << ", " ;
                strm << json_escape_string(it->first) << ": " ;
                it->second.toJSON(strm) ;
                ++it ;
            }
            strm << "}" ;
            break ;
        }
        case Type::Array: {
            strm << "[" ;
            auto it = data_.a_.cbegin() ;
            if ( it != data_.a_.cend() ) {
                it->toJSON(strm) ;
                ++it ;
            }
            while ( it != data_.a_.cend() ) {
                strm << ", " ;
                it->toJSON(strm) ;
                ++it ;
            }

            strm << "]" ;
            break ;
        }
        case Type::String: {
            strm << json_escape_string(data_.s_) ;
            break ;
        }
        case Type::Boolean: {
            strm << ( data_.b_ ? "true" : "false") ;
            break ;
        }
        case Type::Null: {
            strm << "null" ;
            break ;
        }
        case Type::Float: {
            strm << data_.f_ ;
            break ;
        }
        case Type::SInteger: {
            strm << data_.si_ ;
            break ;
        }
        case Type::UInteger: {
            strm << data_.ui_ ;
            break ;
        }
        }
    }

    std::string toJSON() const {
        std::ostringstream strm ;
        toJSON(strm) ;
        return strm.str() ;
    }


    // iterates dictionaries or arrays

    class iterator {
    public:
        iterator(const Variant &obj, bool set_to_begin = false): obj_(obj) {
            if ( obj.isObject() ) o_it_ = ( set_to_begin ) ? obj_.data_.o_.begin() : obj_.data_.o_.end();
            else if ( obj.isArray() ) a_it_ = ( set_to_begin ) ? obj_.data_.a_.begin() : obj_.data_.a_.end();
        }

        const Variant & operator*() const {
            if ( obj_.isObject() ) {
                assert( o_it_ != obj_.data_.o_.end() ) ;
                return o_it_->second ;
            }
            else if ( obj_.isArray() ) {
                assert( a_it_ != obj_.data_.a_.end() ) ;
                return *a_it_ ;
            }
            else {

                return Variant::undefined() ;
            }
        }

        const Variant *operator->() const {
            if ( obj_.isObject() ) {
                assert( o_it_ != obj_.data_.o_.end() ) ;
                return &(o_it_->second) ;
            }
            else if ( obj_.isArray() ) {
                assert( a_it_ != obj_.data_.a_.end() ) ;
                return &(*a_it_) ;
            }
            else {
                return &Variant::undefined() ;
            }
        }

        iterator const operator++(int) {
            iterator it = *this;
            ++(*this);
            return it;
        }


        iterator& operator++() {
            if ( obj_.isObject() ) ++o_it_ ;
            else if ( obj_.isArray() ) ++a_it_ ;

            return *this ;
        }

        iterator const operator--(int) {
            iterator it = *this;
            --(*this);
            return it;
        }


        iterator& operator--() {
            if ( obj_.isObject() ) --o_it_ ;
            else if ( obj_.isArray() ) --a_it_ ;

            return *this ;
        }

        bool operator==(const iterator& other) const
        {
            assert(&obj_ == &other.obj_) ;

            if ( obj_.isObject() ) return (o_it_ == other.o_it_) ;
            else if ( obj_.isArray() ) return (a_it_ == other.a_it_) ;
            else return true ;
        }

        bool operator!=(const iterator& other) const {
            return ! operator==(other);
        }

        std::string key() const {
            if ( obj_.isObject() ) return o_it_->first ;
            else return std::string() ;
        }

        const Variant &value() const {
            return operator*();
        }

    private:
        const Variant &obj_ ;
        Object::const_iterator o_it_ ;
        Array::const_iterator a_it_ ;
    } ;

    iterator begin() const {
        return iterator(*this, true) ;
    }

    iterator end() const {
        return iterator(*this, false) ;
    }

    static const Variant &null() {
        static Variant null_value ;
        return null_value ;
    }

    static const Variant &undefined() {
        static Variant undefined_value ;
        undefined_value.tag_ = Type::Undefined ;
        return undefined_value ;
    }

    Variant invoke(const Variant &args, twig::TemplateEvalContext &ctx) {
        if ( tag_ != Type::Function ) return undefined() ;
        else return (data_.fp_)(args, ctx) ;
    }

private:


    // Original: https://gist.github.com/kevinkreiser/bee394c60c615e0acdad

    static std::string json_escape_string(const std::string &str) {
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



    const Variant &fetchKey(const std::string &key) const {
        assert(isObject()) ;

        auto it = data_.o_.find(key) ;
        if ( it == data_.o_.end() ) return undefined() ; // return undefined
        else return it->second ; // return reference
    }

    const Variant &fetchIndex(uint idx) const {
        assert(isArray()) ;

        if ( idx < data_.a_.size() ) {
            const Variant &v = (data_.a_)[idx] ;
            return v ; // reference to item
        }
        else return undefined() ;
    }

    void destroy() {
        switch (tag_) {
        case Type::Object:
            data_.o_.~Object() ;
            break ;
        case Type::Array:
            data_.a_.~Array() ;
            break ;
        case Type::String:
            data_.s_.~string_t() ;
            break ;
        case Type::Function:
            data_.fp_.~Function() ;
            break ;
        }
    }

    void create(const Variant &other) {
        tag_ = other.tag_ ;
        switch (tag_)
        {
        case Type::Object:
            new ( &data_.o_ ) Object(other.data_.o_) ;
            break;
        case Type::Array:
            new ( &data_.a_ ) Array(other.data_.a_) ;
            break;
        case Type::String:
            new ( &data_.s_ ) string_t(other.data_.s_) ;
            break;
        case Type::Function:
            new ( &data_.fp_ ) Function(other.data_.fp_) ;
            break;
        case Type::Boolean:
            data_.b_ = other.data_.b_ ;
            break;
        case Type::SInteger:
            data_.si_ = other.data_.si_ ;
            break;
        case Type::UInteger:
            data_.ui_ = other.data_.ui_ ;
            break;
        case Type::Float:
            data_.f_ = other.data_.f_ ;
            break;
        default:
            break;
        }

    }



private:

    union Data {
        Object    o_;
        Array     a_ ;
        string_t  s_ ;
        boolean_t   b_ ;
        signed_integer_t   si_ ;
        unsigned_integer_t ui_ ;
        float_t     f_ ;
        Function fp_ ;

        Data() {}
        ~Data() {}
    } ;

    Data data_ ;
    Type tag_ ;


};

class JSONParseException: public std::exception {
public:

    JSONParseException(const std::string &msg, uint line, uint col) {
        std::ostringstream strm ;
        strm << msg << ", at line " << line << ", column " << col ;
        msg_ = strm.str() ;
    }

    const char *what() const noexcept override {
        return msg_.c_str() ;
    }
protected:
    std::string msg_ ;
};



}}
#endif
