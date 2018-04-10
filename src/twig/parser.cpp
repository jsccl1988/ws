#include "parser.hpp"

#include <wspp/twig/exceptions.hpp>

#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <errno.h>

using namespace std;
using namespace wspp::twig::detail ;

TwigParser::TwigParser(std::istream &strm) :
    scanner_(strm),
    parser_(*this, loc_)
{}

void TwigParser::parse(DocumentNodePtr root, const std::string &name) {
  //      parser_.set_debug_level(14);

    root_ = root ;
    stack_.push_back(root);

    loc_.initialize() ;
    script_ = name ;
    parser_.parse();
}


void TwigParser::error(const yy::Parser::location_type &loc, const std::string& m) {
    stringstream strm ;
    strm << script_ << ": " << m << " near " << loc ;
    throw TemplateCompileException(strm.str()) ;
}

void TwigParser::addNode(ContentNodePtr node) {
    trimWhiteBefore() ;
    current_ = node ;

    stack_.back()->addChild(node) ;
}

void TwigParser::popBlock(const char *start_block_name) {

    trimWhiteBefore() ;

    auto it = stack_.rbegin() ;

    current_ = nullptr ;
    while ( it != stack_.rend() ) {
        if ( (*it)->tagName() == start_block_name ) {
            stack_.pop_back() ; return ;
        } else if ( !(*it)->shouldClose() ) {
            ++it ; stack_.pop_back() ;
        }
        else throw TemplateCompileException(str(boost::format("%s: unmatched tag (%s)") % script_ % (*it)->tagName())) ;
    }

}

void TwigParser::trimWhiteBefore()
{
    if ( !current_ ) return ;
    scanner_.trim_next_raw_block_ = false ;
    if ( scanner_.trim_previous_raw_block_ ) {
        if ( RawTextNode *p = dynamic_cast<RawTextNode *>(current_.get()) ) {
            boost::trim_right(p->text_) ;
            if ( p->text_.empty() ) { // erase child
                stack_.back()->children_.pop_back() ;
            }
        }

    }
    scanner_.trim_previous_raw_block_ = false ;

}

void TwigParser::trimWhiteAfter() {
    scanner_.trim_next_raw_block_ = true ;
}


