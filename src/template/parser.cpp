#include "parser.hpp"

#include <boost/regex.hpp>
#include <boost/format.hpp>

#include <errno.h>

using namespace std;

TemplateParser::TemplateParser(std::istream &strm) :
    scanner_(strm),
    parser_(*this, loc_)
{}

bool TemplateParser::parse(ast::DocumentNodePtr root) {

//    parser_.set_debug_level(14);

    root_ = root ;
    stack_.push_back(root);

    loc_.initialize() ;
    int res = parser_.parse();

    return ( res == 0 ) ;
}


void TemplateParser::error(const yy::Parser::location_type &loc, const std::string& m)
{
    std::stringstream strm ;

    strm << m << " near " << loc ;
    error_string_ = strm.str() ;
}


