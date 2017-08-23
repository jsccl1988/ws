#ifndef __WSPP_JSON_HPP__
#define __WSPP_JSON_HPP__

namespace wspp {

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

}
#endif
