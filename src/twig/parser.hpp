#ifndef __TWIG_PARSER_HPP__
#define __TWIG_PARSER_HPP__

#include <deque>
#include <string>
#include <vector>
#include <sstream>
#include <map>

#include <regex>

#include <twig_parser/bison_parser.hpp>
#include "scanner.hpp"
#include "ast.hpp"

#include <wspp/twig/loader.hpp>
#include <wspp/twig/exceptions.hpp>

#include <boost/thread/mutex.hpp>

namespace yy {
class Parser ;
}

class TwigParser {

public:

    TwigParser(std::istream &strm)  ;

    using DocumentNodePtr = wspp::twig::detail::DocumentNodePtr ;
    using ContentNodePtr  = wspp::twig::detail::ContentNodePtr ;
    using ContainerNodePtr  = wspp::twig::detail::ContainerNodePtr ;

    void parse(DocumentNodePtr root, const std::string &name) ;

    void error(const yy::Parser::location_type &loc,  const std::string& m) ;

    void addNode(ContentNodePtr node) {
        scanner_.trim_next_raw_block_ = false ;
        if ( scanner_.trim_previous_raw_block_ ) trimWhiteBefore() ;
        current_ = node ;

        stack_.back()->addChild(node) ;
    }

    void pushBlock(ContainerNodePtr node) {
        stack_.push_back(node) ;
    }

    void popBlock() {
        stack_.pop_back() ;
    }


    void addMacroBlock(const std::string &name, ContentNodePtr node) {
        root_->macro_blocks_.insert({name, node}) ;
    }

    void trimWhiteBefore() ;
    void trimWhiteAfter() ;

    ContainerNodePtr stackTop() const { return stack_.back() ; }

    TwigScanner &scanner() { return scanner_ ; }


private:
    friend class yy::Parser ;

    TwigScanner scanner_;
    yy::Parser parser_;

    std::string error_string_, script_ ;
    yy::Parser::location_type loc_ ;

    DocumentNodePtr root_ ;
    std::deque<ContainerNodePtr> stack_ ;
    ContentNodePtr current_ ;



} ;

#endif
