#ifndef __XML_HPP__
#define __XML_HPP__

#include <istream>
#include <map>
#include <string>
#include <iostream>
#include <memory>

#include <wspp/util/dictionary.hpp>

// Very simple SAX like XML parser

namespace wspp { namespace util {

class XMLStreamWrapper ;

class XMLSAXParser {
public:
    enum ErrorCode { InvalidChar, NoClosingQuote, InvalidHeader, TagMismatch, TagInvalid, AttrValueInvalid, InvalidXml } ;

    XMLSAXParser(const std::string &src) ;

    // parse input stream and return true if succesfull, errors are reported through an error callback

    bool parse() ;

    typedef wspp::util::Dictionary AttributeList ;

    virtual void startElement(const std::string &qname, const AttributeList &attr_list) {
        std::cout << "start element: " << qname << std::endl ;
        for ( const auto &p: attr_list )
            std::cout << "key: " << p.first << ", value: " << p.second << std::endl ;
    }
    virtual void endElement(const std::string &qname) {
        std::cout << "end element: " << qname << std::endl ;
    }

    virtual void characters(const std::string &text_data) {
        std::cout << "characters: " << text_data << std::endl ;
    }

    virtual void error(ErrorCode code, uint line, uint column ) {
        std::cout << "error near line: " << line << ", col: " << column << ", code: " << code << std::endl ;
    }

private:

    struct Cursor {
        Cursor(const std::string &src): cursor_(src.begin()), end_(src.end()) {}

        operator bool () const { return cursor_ != end_ ; }
        char operator * () const { return *cursor_ ; }
        Cursor& operator++() { advance(); return *this ; }
        Cursor operator++(int) {
            Cursor p(*this) ;
            advance() ;
            return p ;
        }

        void advance() {
            // skip new line characters
            column_++ ;

            if ( cursor_ != end_ && *cursor_ == '\r' ) ++cursor_ ;
            if ( cursor_ != end_ && *cursor_ == '\n' ) {
                column_ = 1 ; line_ ++ ;
            }
            cursor_ ++ ;
        }

        std::string::const_iterator cursor_, end_ ;
        uint column_ = 1;
        uint line_ = 1;
    } ;

    bool parseXmlDecl() ;
    bool parseAttributeList(AttributeList &at) ;
    bool parseName(std::string &name);
    bool parseAttributeValue(std::string &val);
    bool parseMisc() ;
    bool parseComment() ;
    bool parseElement() ;
    bool parsePI() ;
    bool parseContent() ;
    bool parseCData() ;
    bool parseCharacters() ;
    bool parseDocType() ;
    bool fatal(ErrorCode code) ;

    void skipSpace() ;
    bool expect(char c) ;
    bool expect(const char *str) ;
    bool escapeString(std::string &value);

    const std::string &src_ ;
    Cursor cursor_ ;

};

} // namespace util
} // namespace wspp
#endif
