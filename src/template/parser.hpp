#ifndef __TEMPLATE_PARSER_HPP__
#define __TEMPLATE_PARSER_HPP__



#include <deque>
#include <string>
#include <vector>
#include <sstream>
#include <map>

#include <regex>

#include <template_parser/bison_parser.hpp>
#include "scanner.hpp"

namespace yy {
class Parser ;
}

class TemplateParser {

public:

    TemplateParser(std::istream &strm)  ;

    bool parse() ;

    void error(const yy::Parser::location_type &loc,  const std::string& m) ;

    TemplateScanner scanner_;
    yy::Parser parser_;

    std::string error_string_, script_ ;
    yy::Parser::location_type loc_ ;
 } ;

#endif
