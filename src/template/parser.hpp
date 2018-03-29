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
#include "template_loader.hpp"
#include "template_exceptions.hpp"

#include <boost/thread/mutex.hpp>

namespace yy {
class Parser ;
}



class TemplateParser {

public:

    TemplateParser(std::istream &strm)  ;

    void parse(detail::DocumentNodePtr root, const std::string &name) ;

    void error(const yy::Parser::location_type &loc,  const std::string& m) ;

    void addNode(detail::ContentNodePtr node) {
        stack_.back()->addChild(node) ;
    }

    void pushBlock(detail::ContainerNodePtr node) {
        stack_.push_back(node) ;
    }

    void popBlock() {
        stack_.pop_back() ;
    }


    void addMacroBlock(const std::string &name, detail::ContentNodePtr node) {
        root_->macro_blocks_.insert({name, node}) ;
    }

    detail::ContainerNodePtr stackTop() const { return stack_.back() ; }


    TemplateScanner &scanner() { return scanner_ ; }



private:
    friend class yy::Parser ;

    TemplateScanner scanner_;
    yy::Parser parser_;

    std::string error_string_, script_ ;
    yy::Parser::location_type loc_ ;

    detail::DocumentNodePtr root_ ;
    std::deque<detail::ContainerNodePtr> stack_ ;


} ;

#endif
