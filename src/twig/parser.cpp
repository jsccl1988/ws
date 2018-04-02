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
//    parser_.set_debug_level(14);

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

void TwigParser::trimWhiteBefore()
{
    if ( !current_ ) return ;
    if ( RawTextNode *p = dynamic_cast<RawTextNode *>(current_.get()) ) {
        boost::trim_right(p->text_) ;
    }
    scanner_.trim_previous_raw_block_ = false ;

}

void TwigParser::trimWhiteAfter()
{
    scanner_.trim_next_raw_block_ = true ;
}


