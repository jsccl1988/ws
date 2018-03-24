%{

#include "parser.hpp"

#include "bison_parser.hpp"


#include <string.h>

void comment() ;
%}

%option debug noyywrap
%option yyclass="TemplateScanner"
%option c++
%option debug
%option case-insensitive

%x COMMENT TAG LUA

%{
# define YY_USER_ACTION  yylloc->columns (yyleng);
        using namespace std ;
%}
%%

%{
   yylloc->step();

%}

<TAG>"AND"                 { return yy::Parser::make_AND(*yylloc); }
<TAG>"&&"                { return yy::Parser::make_AND(*yylloc) ; }
<TAG>"OR"                  { return yy::Parser::make_OR(*yylloc); }
<TAG>"||"                { return yy::Parser::make_OR(*yylloc); }
<TAG>"IN"				{ return yy::Parser::make_IN(*yylloc); }
<TAG>"NOT"				{ return yy::Parser::make_NOT(*yylloc); }

        /* punctuation */

<TAG>"="    { return yy::Parser::make_ASSIGN(*yylloc) ; }
<TAG>"=="    { return yy::Parser::make_EQUAL(*yylloc) ; }
<TAG>"<>"    { return yy::Parser::make_NOT_EQUAL(*yylloc) ; }
<TAG>"!="    { return yy::Parser::make_NOT_EQUAL(*yylloc) ; }
<TAG>"<"     { return yy::Parser::make_LESS_THAN(*yylloc) ; }
<TAG>">"     { return yy::Parser::make_GREATER_THAN(*yylloc) ; }
<TAG>"<="	{ return yy::Parser::make_LESS_THAN_OR_EQUAL(*yylloc) ; }
<TAG>">="	{ return yy::Parser::make_GREATER_THAN_OR_EQUAL(*yylloc) ; }
<TAG>"~"     { return yy::Parser::make_MATCHES(*yylloc) ; }
<TAG>"!~"    { return yy::Parser::make_NOT_MATCHES(*yylloc) ; }
<TAG>"!"     { return yy::Parser::make_NOT(*yylloc) ; }
<TAG>"^"		{ return yy::Parser::make_EXISTS(*yylloc) ; }
<TAG>"("     { return yy::Parser::make_LPAR(*yylloc) ; }
<TAG>")"     { return yy::Parser::make_RPAR(*yylloc) ; }
<TAG>"+"     { return yy::Parser::make_PLUS(*yylloc) ; }
<TAG>"-"     { return yy::Parser::make_MINUS(*yylloc) ; }
<TAG>"*"     { return yy::Parser::make_STAR(*yylloc) ; }
<TAG>"/"     { return yy::Parser::make_DIV(*yylloc) ; }
<TAG>","     { return yy::Parser::make_COMMA(*yylloc) ; }
<TAG>"."     { return yy::Parser::make_DOT(*yylloc) ; }
<TAG>"{"		{ return yy::Parser::make_LEFT_BRACE(*yylloc) ; }
<TAG>"}"		{ return yy::Parser::make_RIGHT_BRACE(*yylloc) ; }

<TAG>"]"		{ return yy::Parser::make_RIGHT_BRACKET(*yylloc) ; }
<TAG>"["	    { return yy::Parser::make_LEFT_BRACKET(*yylloc) ; }
<TAG>":"	    { return yy::Parser::make_COLON(*yylloc) ; }

<TAG>[a-zA-Z][a-zA-Z0-9_]* {
        return yy::Parser::make_IDENTIFIER(yytext, *yylloc);
}

<TAG>[0-9]+	|
[0-9]+"."[0-9]* |
"."[0-9]*	|
[0-9]+[eE][+-]?[0-9]+	|
[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
"."[0-9]*[eE][+-]?[0-9]+	{
    return yy::Parser::make_NUMBER(atof(yytext), *yylloc) ;
}

<TAG>['\"]([^\\\']|\\.)*['\"] {
    return yy::Parser::make_STRING(std::string(yytext+1, yyleng-2), *yylloc) ;
}

[\n]+   yylloc->lines (yyleng); yylloc->step ();

[ \t]+   {yylloc->step ();}

<<EOF>>    return yy::Parser::make_END(*yylloc);

<TAG>"/*" { BEGIN(COMMENT); }
<COMMENT>\n { yylloc->lines (); yylloc->step ();}
<COMMENT>"*/" { BEGIN(TAG) ; }
<COMMENT>.  { yylloc->step (); }

"#"[^\n]*\n { yylloc->step();}

"<%" { BEGIN(TAG); return yy::Parser::make_START_TAG(*yylloc) ; }
<TAG>"%>" { BEGIN(INITIAL) ;  return yy::Parser::make_END_TAG(*yylloc) ; }

<INITIAL>. { raw_.push_back(*yytext) ;}
%%


