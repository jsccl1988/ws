#ifndef __WSPP_UTIL_XML_WRITER_HPP__
#define __WSPP_UTIL_XML_WRITER_HPP__

#include <iostream>
#include <stack>
#include <cassert>

namespace wspp { namespace util {

class XmlWriter {
public:
    XmlWriter(std::ostream &out): strm_(out) {}

    XmlWriter &startDocument(const std::string &xml_version = "1.0", const std::string &encoding = "utf-8");

    XmlWriter &setIndent(bool indent) { indent_ = indent ; return *this ; }
    XmlWriter &setIndentString(const std::string istr) { indent_str_ = istr ; return *this ; }

    XmlWriter &startElement(const std::string &name) ;
    XmlWriter &endElement() ;

    XmlWriter &attr(const std::string &key, const std::string &value) ;

    // text is inline with start/end elements
    XmlWriter &text(const std::string &text) ;

    // this adds new line before and after the text
    XmlWriter &content(const std::string &text) ;

    void endDocument() {
        assert(elements_.empty()) ;
        strm_.flush() ;
    }

private:

    std::ostream & strm_;
    bool indent_ = true ;
    std::string indent_str_ = "    " ;
    bool tag_open_ = false ;
    bool inline_ = false ;

    std::stack<std::string> elements_;

    void indent();
    void toggleTag() ;
    void write_escaped(const std::string &str);
};



} // util
} // wspp

#endif
