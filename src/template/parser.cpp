#include "parser.hpp"

#include <boost/regex.hpp>
#include <boost/format.hpp>

#include <errno.h>

using namespace std;

TemplateParser::TemplateParser(std::istream &strm) :
    scanner_(strm),
    parser_(*this, loc_)
{}

bool TemplateParser::parse() {
    //  parser_.set_debug_level(14);

       stack_.push_back(std::shared_ptr<ast::ContainerNode>(new ast::DocumentNode()));
    loc_.initialize() ;
    int res = parser_.parse();

    return ( res == 0 ) ;
}

void TemplateParser::eval(ast::TemplateEvalContext &ctx, std::string &s)
{
    stack_.back()->eval(ctx, s) ;
}

void TemplateParser::error(const yy::Parser::location_type &loc, const std::string& m)
{
    std::stringstream strm ;

    strm << m << " near " << loc ;
    error_string_ = strm.str() ;
}


