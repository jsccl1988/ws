#ifndef __TEMPLATE_SCANNER_HPP__
#define __TEMPLATE_SCANNER_HPP__

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include <parser.hpp>
#include <template_parser/bison_parser.hpp>

#undef YY_DECL
#define YY_DECL                        \
    yy::Parser::symbol_type            \
    TemplateScanner::lex(              \
    yy::Parser::location_type* yylloc  \
    )


class TemplateScanner : public yyFlexLexer{

public:
    TemplateScanner(std::istream &strm): yyFlexLexer(&strm)  {}

    virtual yy::Parser::symbol_type lex(yy::Parser::location_type* yylloc);

    std::string raw_ ;
};



#endif
