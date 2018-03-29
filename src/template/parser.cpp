#include "parser.hpp"

#include "template_exceptions.hpp"

#include <boost/regex.hpp>
#include <boost/format.hpp>

#include <errno.h>

using namespace std;

TemplateParser::TemplateParser(std::istream &strm) :
    scanner_(strm),
    parser_(*this, loc_)
{}

void TemplateParser::parse(detail::DocumentNodePtr root, const std::string &name) {
//    parser_.set_debug_level(14);

    root_ = root ;
    stack_.push_back(root);

    loc_.initialize() ;
    script_ = name ;
    parser_.parse();
}


void TemplateParser::error(const yy::Parser::location_type &loc, const std::string& m) {
    stringstream strm ;
    strm << script_ << ": " << m << " near " << loc ;
    throw TemplateCompileException(strm.str()) ;
}


