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


namespace wspp { namespace util {

class Variant {

public:

    using Object = std::map<std::string, Variant> ;
    using Array = std::vector<Variant> ;

    using signed_integer_t = int64_t ;
    using unsigned_integer_t = uint64_t ;
    using float_t = double ;
    using string_t = std::string ;
    using boolean_t = bool ;

    // constructors

    Variant(): tag_(Tag::Null) {}

    Variant(boolean_t v) noexcept : tag_(Tag::Boolean), b_(v) { }

    Variant(int v) noexcept: Variant((int64_t)v) {}
    Variant(unsigned int v) noexcept: Variant((uint64_t)v) {}

    Variant(int64_t v) noexcept {
        if ( v < 0 ) {
            tag_ = Tag::SInteger ;
            si_ = v ;
        } else {
            tag_ = Tag::UInteger ;
            ui_ = v ;
        }
    }

    Variant(uint64_t v) noexcept: tag_(Tag::UInteger), ui_(v) {}

    Variant(float_t v) noexcept: tag_(Tag::Float), f_(v) { }

    Variant(const char *value): tag_(Tag::String) {
        s_ = new std::string(value) ;
    }

    Variant(const string_t& value): tag_(Tag::String) {
        s_ = new std::string(value) ;
    }

    Variant(string_t&& value): tag_(Tag::String)  {
        s_ = new std::string(std::move(value)) ;
    }

    Variant(const Object& value): tag_(Tag::Object) {
        o_ = new Object(value);
    }

    Variant(Object&& value): tag_(Tag::Object) {
        o_ = new Object(std::move(value));
    }

    Variant(const Array& value): tag_(Tag::Array) {
        a_ = new Array(value);
    }

    /// constructor for rvalue arrays
    Variant(Array&& value): tag_(Tag::Array) {
        a_ = new Array(std::move(value));
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
        case Tag::Object:
            o_ = other.o_ ;
            break;
        case Tag::Array:
            a_ = other.a_ ;
            break;
        case Tag::String:
            s_ = other.s_ ;
            break;
        case Tag::Boolean:
            b_ = other.b_ ;
            break;
        case Tag::SInteger:
            si_ = other.si_ ;
            break;
        case Tag::UInteger:
            ui_ = other.ui_ ;
            break;
        case Tag::Float:
            f_ = other.f_ ;
            break;
        default:
            break;
        }
        other.tag_ = Tag::Null ;
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
    bool isObject() const { return tag_ == Tag::Object ; }
    bool isArray() const { return tag_ == Tag::Array ; }
    bool isNull() const { return tag_ == Tag::Null ; }

    // check if variant stores simple type string, number, integer or boolean
    bool isPrimitive() const {
        return ( tag_ == Tag::String ||
                 tag_ == Tag::SInteger ||
                 tag_ == Tag::UInteger ||
                 tag_ == Tag::Float ||
                 tag_ == Tag::Boolean
                 ) ;
    }

    // False are booleans with false values and empty arrays
    bool isFalse() const {
        if ( tag_ == Tag::Boolean ) return !b_ ;
        else if ( tag_ == Tag::Array ) return a_->size() == 0 ;
        else return false ;
    }

    // convert value to string
    std::string toString() const {
        switch (tag_)
        {
        case Tag::String:
            return *s_;
        case Tag::Boolean: {
            std::ostringstream strm ;
            strm << b_ ;
            return strm.str() ;
        }
        case Tag::SInteger:
            return std::to_string(si_) ;
        case Tag::UInteger:
            return std::to_string(ui_) ;
        case Tag::Float:
            return std::to_string(f_) ;
        default:
            return std::string();
        }
    }

    // Return the keys of an Object otherwise an empty list
    std::vector<std::string> keys() const {
        std::vector<std::string> res ;

        if ( !isObject() ) return res ;

        for ( const auto &p: *o_ )
            res.push_back(p.first) ;
        return res ;
    }

    // length of object or array, zero otherwise
    size_t length() const {
        if ( isObject() )
            return o_->size() ;
        else if ( isArray() ) {
            return a_->size() ;
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
        if ( !current->isObject() ) return null() ;

        for ( tokenizer::iterator it = tokens.begin(); it != tokens.end();  ) {
            const Variant &val = current->fetchKey(*it++) ;
            if ( val.isNull() ) return val ;
            else if ( it != tokens.end() )
                current = &val ;
            else return val ;
        }

        return Variant::null() ;
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

    // JSON encoder
    void toJSON(std::ostream &strm) const {

        switch ( tag_ ) {
        case Tag::Object: {
            strm << "{" ;
            auto it = o_->cbegin() ;
            if ( it != o_->cend() ) {
                strm << json_escape_string(it->first) << ": " ;
                it->second.toJSON(strm) ;
                ++it ;
            }
            while ( it != o_->cend() ) {
                strm << ", " ;
                strm << json_escape_string(it->first) << ": " ;
                it->second.toJSON(strm) ;
                ++it ;
            }
            strm << "}" ;
            break ;
        }
        case Tag::Array: {
            strm << "[" ;
            auto it = a_->cbegin() ;
            if ( it != a_->cend() ) {
                it->toJSON(strm) ;
                ++it ;
            }
            while ( it != a_->cend() ) {
                strm << ", " ;
                it->toJSON(strm) ;
                ++it ;
            }

            strm << "]" ;
            break ;
        }
        case Tag::String: {
            strm << json_escape_string(*s_) ;
            break ;
        }
        case Tag::Boolean: {
            strm << ( b_ ? "true" : "false") ;
            break ;
        }
        case Tag::Null: {
            strm << "null" ;
            break ;
        }
        case Tag::Float: {
            strm << f_ ;
            break ;
        }
        case Tag::SInteger: {
            strm << si_ ;
            break ;
        }
        case Tag::UInteger: {
            strm << ui_ ;
            break ;
        }
        }

    }

    std::string toJSON() const {
        std::ostringstream strm ;
        toJSON(strm) ;
        return strm.str() ;
    }




    class iterator {
    public:
        iterator(const Variant &obj, bool set_to_begin = false): obj_(obj) {
            if ( obj.isObject() ) o_it_ = ( set_to_begin ) ? obj_.o_->begin() : obj_.o_->end();
            else if ( obj.isArray() ) a_it_ = ( set_to_begin ) ? obj_.a_->begin() : obj_.a_->end();
        }

        const Variant & operator*() const {
            if ( obj_.isObject() ) {
                assert( o_it_ != obj_.o_->end() ) ;
                return o_it_->second ;
            }
            else if ( obj_.isArray() ) {
                assert( a_it_ != obj_.a_->end() ) ;
                return *a_it_ ;
            }
            else {

                return Variant::null() ;
            }
        }

        const Variant *operator->() const {
            if ( obj_.isObject() ) {
                assert( o_it_ != obj_.o_->end() ) ;
                return &(o_it_->second) ;
            }
            else if ( obj_.isArray() ) {
                assert( a_it_ != obj_.a_->end() ) ;
                return &(*a_it_) ;
            }
            else {
                return &Variant::null() ;
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

        auto it = o_->find(key) ;
        if ( it == o_->end() ) return null() ; // return null
        else return it->second ; // return reference
    }

    const Variant &fetchIndex(uint idx) const {
        assert(isArray()) ;

        if ( idx < a_->size() ) {
            const Variant &v = (*a_)[idx] ;
            return v ; // reference to item
        }
        else return null() ;
    }

    void destroy() {
        switch (tag_) {
        case Tag::Object:
            delete o_ ;
            break ;
        case Tag::Array:
            delete a_ ;
            break ;
        case Tag::String:
            delete s_ ;
            break ;
        }
    }

    void create(const Variant &other) {

        tag_ = other.tag_ ;
        switch (tag_)
        {
        case Tag::Object:
            o_ = new Object(*other.o_) ;
            break;
        case Tag::Array:
            a_ = new Array(*other.a_) ;
            break;
        case Tag::String:
            s_ = new string_t(*other.s_) ;
            break;
        case Tag::Boolean:
            b_ = other.b_ ;
            break;
        case Tag::SInteger:
            si_ = other.si_ ;
            break;
        case Tag::UInteger:
            ui_ = other.ui_ ;
            break;
        case Tag::Float:
            f_ = other.f_ ;
            break;
        default:
            break;
        }

    }

private:
    enum class Tag : uint8_t {
        Null,  Object, Array, String, Boolean, SInteger, UInteger, Float
    };

    union {
        Object    *o_;
        Array     *a_ ;
        string_t  *s_ ;
        boolean_t   b_ ;
        signed_integer_t   si_ ;
        unsigned_integer_t ui_ ;
        float_t     f_ ;
        const Variant *r_ ;
    } ;

    Tag tag_ ;


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
