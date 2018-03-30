#ifndef __TWIG_SCANNER_HPP__
#define __TWIG_SCANNER_HPP__

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "parser.hpp"
#include <twig_parser/bison_parser.hpp>

#undef YY_DECL
#define YY_DECL                        \
    yy::Parser::symbol_type            \
    TwigScanner::lex(              \
    yy::Parser::location_type* yylloc  \
    )


class TwigScanner : public yyFlexLexer{

public:
    TwigScanner(std::istream &strm): yyFlexLexer(&strm)  {}

    virtual yy::Parser::symbol_type lex(yy::Parser::location_type* yylloc);

    std::string raw_ ;
};



#endif
