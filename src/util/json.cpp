// json decoder

#include <wspp/util/variant.hpp>
#include <wspp/util/filesystem.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include <fstream>

using namespace std ;
using namespace wspp::util ;


namespace wspp { namespace util {
JSONParseException::JSONParseException(const string &msg, uint line, uint col) {
    ostringstream strm ;
    strm << msg << ", at line " << line << ", column " << col ;
    msg_ = strm.str() ;
}
}}

class JSONParser {
public:
    JSONParser(const string &src): src_(src), pos_(src) {}

    bool parse(Variant &val) ;

private:

    struct Position {
        Position(const std::string &src): cursor_(src.begin()), end_(src.end()) {}

        operator bool () const { return cursor_ != end_ ; }
        char operator * () const { return *cursor_ ; }
        Position& operator++() { advance(); return *this ; }
        Position operator++(int) {
            Position p(*this) ;
            advance() ;
            return p ;
        }

        void advance() {
            // skip new line characters
            column_++ ;

            if ( cursor_ != end_ && *cursor_ == '\n' ) {
                column_ = 1 ; line_ ++ ;
            }

            cursor_ ++ ;

        }

        string::const_iterator cursor_, end_ ;
        uint column_ = 1;
        uint line_ = 1;
    } ;

    const string &src_ ;
    Position pos_ ;

private:

    bool parseValue(Variant &val) ;
    bool parseString(Variant &val) ;
    bool parseNumber(Variant &val) ;
    bool parseArray(Variant &val) ;
    bool parseObject(Variant &val) ;
    bool parseBoolean(Variant &val) ;
    bool parseNull(Variant &val) ;
    bool parseKeyValuePair(string &key, Variant &val) ;

    void skipSpace() ;
    bool expect(char c) ;
    bool expect(const char *str) ;
    bool decodeUnicode(uint &cp) ;
    static string unicodeToUTF8(uint cp) ;

    bool throwException(const std::string msg) ;
};


bool JSONParser::parse( Variant &value ) {
    if ( !parseValue(value) ) {
        throwException("Error parsing json value") ;
        return false ;
    }
    return true ;
}

bool JSONParser::parseValue(Variant &val) {
    return
            parseString(val) ||
            parseNumber(val) ||
            parseObject(val) ||
            parseArray(val) ||
            parseBoolean(val) ||
            parseNull(val) ;
}

bool JSONParser::parseString(Variant &val)
{
    string res ;
    bool in_escape = false ;

    if ( !expect("\"") ) return false ;
    while ( pos_ ) {
        char c = *pos_++ ;
        if ( c == '"' ) {
            val = Variant(res) ;
            return true ;
        }
        else if ( c == '\\' ) {
            if ( !pos_ ) return throwException("End of file while parsing string literal");
            char escape = *pos_++ ;

            switch (escape) {
            case '"':
                res += '"'; break ;
            case '/':
                res += '/'; break ;
            case '\\':
                res += '\\'; break;
            case 'b':
                res += '\b'; break ;
            case 'f':
                res += '\f'; break ;
            case 'n':
                res += '\n'; break ;
            case 'r':
                res += '\r'; break ;
            case 't':
                res += '\t'; break ;
            case 'u': {
                unsigned int cp ;
                if ( !decodeUnicode(cp) ) return throwException("Error while decoding unicode code point") ;
                res += unicodeToUTF8(cp);
            } break;
            default:
                return throwException("Invalid character found while decoding string literal") ;
            }

        } else {
            res += c;
        }
    }

    return false ;
}

bool JSONParser::parseNumber(Variant &val) {
    static boost::regex rx_number(R"(^-?(?:0|[1-9]\d*)(?:\.\d+)?(?:[eE][+-]?\d+)?)") ;

    skipSpace() ;

    boost::smatch what ;
    if ( !boost::regex_search(pos_.cursor_, pos_.end_, what, rx_number) ) return false ;
    else {
        string c = what[0] ;
        pos_.cursor_ += c.length() ;

        try {
            int64_t number = boost::lexical_cast<int64_t>(c) ;
            val = Variant(number) ;
        }
        catch ( boost::bad_lexical_cast & ) {
            try {
                float number = boost::lexical_cast<float>(c) ;
                val = Variant(number) ;
            }
            catch ( boost::bad_lexical_cast & ) {
                double number = boost::lexical_cast<double>(c) ;
                val = Variant(number) ;
            }
        }


        return true ;
    }

}

bool JSONParser::parseArray(Variant &val)
{
    Variant::Array elements ;

    if ( !expect("[") ) return false ;
    Variant element ;
    while ( 1 ) {
        if ( expect("]") ) {
            val = elements ;
            return true ;
        }
        else if ( !elements.empty() && !expect(",") ) {
            throwException("Expecting ','") ;
            return false ;
        }

        if ( !parseValue(element) ) break ;
        else elements.emplace_back(element) ;
    } ;

}

bool JSONParser::parseObject(Variant &val)
{
    Variant::Object elements ;

    if ( !expect("{") ) return false ;

    string key ;
    Variant element ;

    while ( 1 ) {
        if ( expect("}") ) {
            val = elements ;
            return true ;
        }
        else if ( !elements.empty() && !expect(",") ) {
            throwException("Expecting ','") ;
            return false ;
        }

        if ( !parseKeyValuePair(key, element) ) break ;
        else {
            if ( key.empty() ) key = "__empty__" ;
            elements.insert({key, element}) ;
        }
    }


}

bool JSONParser::parseBoolean(Variant &val)
{
    if ( expect("true") ) {
        val = Variant(true) ;
        return true ;
    }
    else if ( expect("false") ) {
        val = Variant("false") ;
        return true ;
    }

    return false ;

}

bool JSONParser::parseNull(Variant &val)
{
    if ( expect("null") ) {
        val = Variant() ;
        return true ;
    }

    return false ;
}

bool JSONParser::parseKeyValuePair(string &key, Variant &val) {
    Variant keyv ;
    if ( !parseString(keyv) ) return false ;
    key = keyv.toString() ;
    if ( !expect(":") ) return false ;
    if ( !parseValue(val) ) return false ;
    return true ;
}

void JSONParser::skipSpace() {
    while ( pos_ ) {
        char c = *pos_ ;
        if ( isspace(c) ) ++pos_ ;
        else return ;
   }
}

bool JSONParser::expect(char c) {
    if ( pos_ ) {
        if ( *pos_ == c ) {
            ++pos_ ;
            return true ;
        }
        return false ;
    }
    return false ;
}

bool JSONParser::expect(const char *str)
{
    const char *c = str ;

    skipSpace() ;
    Position cur = pos_ ;
    while ( *c != 0 ) {
        if ( !expect(*c) ) {
            pos_ = cur ;
            return false ;
        }
        else ++c ;
    }
    return true ;
}

// adopted from https://github.com/open-source-parsers/jsoncpp

bool JSONParser::decodeUnicode(uint &cp)
{
    int unicode = 0 ;

    for( uint i=0 ; i<4 ; i++ ) {
        if ( !pos_ ) return false ;
        char c = *pos_++ ;
        unicode *= 16 ;
        if ( c >= '0' && c <= '9')
            unicode += c - '0';
        else if (c >= 'a' && c <= 'f')
            unicode += c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            unicode += c - 'A' + 10;
        else
            return false ;
    }

    cp = static_cast<unsigned int>(unicode);
    return true;
}

bool JSONParser::throwException(const string msg)
{
    throw JSONParseException(msg, pos_.line_, pos_.column_) ;
    return false ;
}

/// Converts a unicode code-point to UTF-8.

string JSONParser::unicodeToUTF8(unsigned int cp) {
    string result ;

    if ( cp <= 0x7f ) {
        result.resize(1);
        result[0] = static_cast<char>(cp);
    } else if ( cp <= 0x7FF ) {
        result.resize(2);
        result[1] = static_cast<char>(0x80 | (0x3f & cp));
        result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
    } else if ( cp <= 0xFFFF ) {
        result.resize(3);
        result[2] = static_cast<char>(0x80 | (0x3f & cp));
        result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
        result[0] = static_cast<char>(0xE0 | (0xf & (cp >> 12)));
    } else if ( cp <= 0x10FFFF ) {
        result.resize(4);
        result[3] = static_cast<char>(0x80 | (0x3f & cp));
        result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
        result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
        result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
    }

    return result;
}

Variant Variant::fromJSONString(const std::string &src, bool throw_exception) {
    JSONParser parser(src) ;

    Variant val ;
    try {
        parser.parse(val) ;
        return val ;
    }
    catch ( JSONParseException &e ) {
        cout << e.what() << endl ;
        if ( throw_exception ) throw e ;
        else return Variant() ;
    }
}

Variant Variant::fromJSONFile(const std::string &fpath, bool throw_exception) {
    return fromJSONString(readFileToString(fpath), throw_exception) ;
}
