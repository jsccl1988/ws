#include <wspp/util/xml_sax_parser.hpp>

#include <iterator>
#include <stack>

using namespace std;

namespace wspp {
namespace util {
class XMLSAXException {
public:
    XMLSAXException(XMLSAXParser::ErrorCode code, uint line, uint col)
        : code_(code), line_(line), column_(col) {}
    XMLSAXParser::ErrorCode code_;
    uint line_, column_;
};

static bool is_valid_name_char(char c) {
    return ( isalnum(c) || c == '-' || c == '_' || c == '.' || c == ':' );
}

bool XMLSAXParser::escapeString(string &value) {
    uint k = 0;
    string quot;
    char c = *cursor_++;
    do {
        quot += c;
        c = *cursor_++;
        ++k;
    } while ( cursor_ && c != ';' && k < 5 );

    if ( quot == "amp" ) value += '&';
    else if ( quot == "quot" ) value += '\"';
    else if ( quot == "lt" ) value += '<';
    else if ( quot == "gt" ) value += '>';
    else if ( quot == "apos") value += '\'';
    else return false;

    return true;
}

bool XMLSAXParser::parseXmlDecl() {
    if ( expect("<?xml") ) {
        skipSpace();
        AttributeList attrs;
        if ( !parseAttributeList(attrs) ) return fatal(InvalidHeader);
        skipSpace();
        if ( !expect("?>") ) return fatal(TagInvalid);
        return true;
    }

    return false;
}

bool XMLSAXParser::parseName(std::string &name) {
    do {
        char c = *cursor_;
        if ( !is_valid_name_char(c) ) break;
        else { name += c; ++cursor_; }
    } while ( cursor_ );

    return !name.empty();
}

bool XMLSAXParser::parseAttributeValue(std::string &val) {
    char c = *cursor_++;
    char oc = c;

    // matching starting quote
    if ( oc != '"' && oc != '\'' ) return false;

    // eat characters (no backtracking here)
    while ( cursor_ ) {
        c = *cursor_;

        if ( c == '&' ) {
            ++cursor_;
            if ( !escapeString(val) ) return false;
        }
        else if ( c == '<' ) return false;
        else if ( c == oc ) {
            cursor_++;
            break;
        }
        else {
            val += c;
            ++cursor_;
        }
    } ;

    // closing quote
    return c == oc;
}

bool XMLSAXParser::parseMisc(){
    bool match = false;
    while ( cursor_ ) {
        skipSpace();
        if ( !parseComment() &&
             !parsePI() ) break;
        match = true;
    }
    skipSpace();
    return match;
}

bool XMLSAXParser::parseComment(){
    if ( expect("<!--") ) {
        char c;
        do {
            c = *cursor_++;
        }
        while ( cursor_ && c != '-' );

        if ( expect("->") ) return true;
        else
            fatal(InvalidChar);
    }

    return false;
}

bool XMLSAXParser::parseElement(){
    Cursor oc = cursor_;

    if ( expect("<") ) {
        char c = *cursor_;
        if ( c == '/' ) {
            cursor_ = oc;
            return false;
        }

        string tag;
        AttributeList at;
        if ( !parseName(tag) ) return fatal(TagInvalid);
        skipSpace();

        parseAttributeList(at);
        if ( expect("/>") ) {
            startElement(tag, at);
            characters("");
            endElement(tag);
            return true;
        }
        else if ( expect(">") ) {
            startElement(tag, at);
            parseContent();
            if ( expect("</") ) {
                string closing_tag;

                if ( !parseName(closing_tag) ) return fatal(TagInvalid);
                if ( tag != closing_tag ) return fatal(TagMismatch);

                skipSpace();
                if ( expect(">") ) {
                    endElement(closing_tag);
                    return true;
                }
            }
        }
        else return fatal(InvalidChar);
    }
    return false;
}

bool XMLSAXParser::parsePI(){
    if ( expect("<?") ) {
        char c;
        while ( cursor_ ) {
            c = *cursor_++;
            if ( c == '>' ) break;
        };
        return true;
    }
    return false;
}

bool XMLSAXParser::parseContent(){
    while ( 1 ) {
        if ( parseCData() ) continue;
        if ( parseComment() ) continue;
        if ( parsePI() ) continue;
        if ( parseElement() ) continue;
        if ( parseCharacters() ) continue;
        break;
    }
    return true;
}

bool XMLSAXParser::parseCData(){
    string text;

    if ( expect("<![CDATA[") ) {
        char c;
        while ( cursor_ ) {
            c = cursor_++;
            if ( c != ']' ) text += c;
            else {
                if ( expect("]>")) {
                    if ( !text.empty() )
                        characters(text);
                    return true;
                }
            }
        }
    }

    return false;
}

bool XMLSAXParser::parseCharacters(){
    string text;
    while ( cursor_ ) {
        char c = *cursor_;
        if ( c == '<' ) break;
        else if ( c == '&' ) {
           if ( !escapeString(text) )
               return fatal(InvalidChar);
        }
        else {
           text += c;
           ++cursor_;
        }
    }

    if ( !text.empty() ) {
        characters(text);
        return true;
    }
    return false;
}

bool XMLSAXParser::parseDocType() {
    if ( expect("<!DOCTYPE") ) {
        // ignore section
        while ( cursor_ ) {
            char c = cursor_++;
            if ( c == '>' ) break;
            else if ( c == '[') { // skip until ending bracket (maybe nested)
                uint depth = 1;
                while ( cursor_ )
                {
                    char c = cursor_++;
                    if ( c == '[' ) ++depth;
                    else if ( c == ']' ) --depth;

                    if ( depth == 0 ) break;
                }
            }
       }

       return true;
    }

    return false;
}

bool XMLSAXParser::parseAttributeList(XMLSAXParser::AttributeList &at){
    do {
        string attrName, attrValue;
        if ( !parseName(attrName) ) return false;
        skipSpace();
        if ( !expect('=') ) fatal(InvalidChar);
        skipSpace();
        if ( !parseAttributeValue(attrValue) ) fatal(AttrValueInvalid);
        at.add(attrName, attrValue);
        skipSpace();
        char c = *cursor_;
        if ( c == '/' || c == '>' || c == '?' ) break;
    } while ( cursor_ );
    return true;
}

bool XMLSAXParser::fatal(ErrorCode code) {
    throw XMLSAXException(code, cursor_.line_, cursor_.column_);
    return false;
}

XMLSAXParser::XMLSAXParser(const string &src): src_(src), cursor_(src) {}

void XMLSAXParser::skipSpace() {
    while ( cursor_ ) {
        char c = *cursor_;
        if ( isspace(c) ) ++cursor_;
        else return;
   }
}

bool XMLSAXParser::expect(char c) {
    if ( cursor_ ) {
        if ( *cursor_ == c ) {
            ++cursor_;
            return true;
        }
        return false;
    }
    return false;
}

bool XMLSAXParser::expect(const char *str) {
    const char *c = str;

    skipSpace();
    Cursor cur = cursor_;
    while ( *c != 0 ) {
        if ( !expect(*c) ) {
            cursor_ = cur;
            return false;
        }
        else ++c;
    }
    return true;
}

bool XMLSAXParser::parse() {
    try {
        // prolog
        if ( !parseXmlDecl() ) return false; // maybe we can make this less strict with option
        parseMisc();
        while ( 1 ) {
            if ( parseDocType() ) continue;
            if ( parseMisc() ) continue;
            break;
        }

        if ( !parseElement() ) fatal(InvalidXml);
        parseMisc();

        return true;
    } catch ( XMLSAXException &e ) {
        error( e.code_, e.line_, e.column_ );
        return false;
    }
}
} // namespace util
} // namespace wspp