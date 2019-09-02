#include <wspp/util/xml_writer.hpp>

using namespace std;

namespace wspp {
namespace util {
XmlWriter &XmlWriter::startDocument(const std::string &xml_version, const std::string &encoding) {
    strm_ << "<?xml version=\"" << xml_version << "\" encoding=\"" << encoding << "\"?>\n";
    return *this;
}

XmlWriter &XmlWriter::startElement(const std::string &name) {
    toggleTag();
    if ( !elements_.empty() ) strm_ << endl;
    indent();
    strm_ << "<" << name;
    elements_.push(name);
    tag_open_ = true;
    return *this;
}

XmlWriter &XmlWriter::endElement() {
    toggleTag();
    string el = elements_.top();
    elements_.pop();
    if ( !inline_ ) {
        strm_ << endl;
        indent();
    }
    inline_ = false;
    strm_ << "</" << el << ">";
    tag_open_ = false;
    return *this;
}

XmlWriter &XmlWriter::attr(const string &key, const string &value) {
    strm_ << " " << key << "=\"";
    write_escaped(value);
    strm_ << '"';
    return *this;
}

XmlWriter &XmlWriter::content(const string &text) {
    toggleTag();
    strm_ << endl;
    indent();
    write_escaped(text);
    return *this;
}

XmlWriter &XmlWriter::text(const string &text) {
    toggleTag();
    write_escaped(text);
    inline_ = true;
    return *this;
}

void XmlWriter::indent() {
    if ( indent_ ) {
        for ( size_t i = 0; i < elements_.size(); i++ )
            strm_ << indent_str_;
    }
}

void XmlWriter::toggleTag() {
    if ( tag_open_ ) {
        strm_ << ">";
        tag_open_ = false;
    }
    else tag_open_ = true;
}

void XmlWriter::write_escaped(const std::string &str) {
    for ( char c: str) {
        switch (c) {
        case '&'  : strm_ << "&amp;"; break;
        case '<'  : strm_ << "&lt;"; break;
        case '>'  : strm_ << "&gt;"; break;
        case '\'' : strm_ << "&apos;"; break;
        case '"'  : strm_ << "&quot;"; break;
        default   : strm_.put(c); break;
        }
    }
}
} // util
} // wspp