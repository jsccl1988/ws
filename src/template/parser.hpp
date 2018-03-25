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
#include "template_ast.hpp"

namespace yy {
class Parser ;
}

class TemplateParser {

public:

    TemplateParser(std::istream &strm)  ;

    bool parse() ;

    wspp::util::Variant eval(ast::TemplateEvalContext &ctx) ;

    void error(const yy::Parser::location_type &loc,  const std::string& m) ;

    void addNode(ast::ContentNodePtr node) {
        stack_.back()->children_.push_back(node) ;
    }

    void pushBlock(ast::ContainerNodePtr node) {
        stack_.push_back(node) ;
    }

    void popBlock() {
        stack_.pop_back() ;
    }


    TemplateScanner scanner_;
    yy::Parser parser_;

    std::string error_string_, script_ ;
    yy::Parser::location_type loc_ ;

    ast::ExpressionNodePtr root_ ;
    std::deque<ast::ContainerNodePtr> stack_ ;
 } ;

#endif
