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

%x COMMENT TAG SUBS TCOMMENT

%{
# define YY_USER_ACTION  yylloc->columns (yyleng);
using namespace std ;

#define TOKEN(name) yy::Parser::make_##name(*yylloc)

%}
%%

%{
   yylloc->step();
%}

<TAG>{
  "block"       return TOKEN(T_BEGIN_BLOCK) ;
  "endblock"    return TOKEN(T_END_BLOCK) ;
  "for"         return TOKEN(T_FOR) ;
  "endfor"      return TOKEN(T_END_FOR) ;
  "elif"        return TOKEN(T_ELSE_IF) ;
  "endif"       return TOKEN(T_END_IF) ;
  "set"         return TOKEN(T_SET) ;
  "endset"      return TOKEN(T_END_SET) ;
  "filter"      return TOKEN(T_FILTER) ;
  "endfilter"   return TOKEN(T_END_FILTER) ;
  "extends"     return TOKEN(T_EXTENDS) ;
  "macro"       return TOKEN(T_MACRO) ;
  "endmacro"    return TOKEN(T_END_MACRO) ;
  "import"      return TOKEN(T_IMPORT) ;
  "as"          return TOKEN(T_AS) ;
  "_self"       return TOKEN(T_SELF) ;
  "in"          return TOKEN(T_IN);
  "%}"          { BEGIN(INITIAL) ;  return TOKEN(T_END_BLOCK_TAG) ; }
  "-%}"         { BEGIN(INITIAL) ;  return TOKEN(T_END_BLOCK_TAG_TRIM) ; }
}

<TAG,SUBS>{

"if"    return TOKEN(T_IF) ;
"else"  return TOKEN(T_ELSE) ;
"and"   return TOKEN(T_AND);
"&&"    return TOKEN(T_AND);
"or"    return TOKEN(T_OR);
"||"    return TOKEN(T_OR);
"not"	return TOKEN(T_NOT);

"true"  return TOKEN(T_TRUE);
"false" return TOKEN(T_FALSE);
"null"  return TOKEN(T_NULL);

"="     return TOKEN(T_ASSIGN) ;
"=="    return TOKEN(T_EQUAL) ;
"<>"    return TOKEN(T_NOT_EQUAL) ;
"!="    return TOKEN(T_NOT_EQUAL) ;
"<"     return TOKEN(T_LESS_THAN) ;
">"     return TOKEN(T_GREATER_THAN) ;
"<="	return TOKEN(T_LESS_THAN_OR_EQUAL) ;
">="	return TOKEN(T_GREATER_THAN_OR_EQUAL) ;
"~"     return TOKEN(T_TILDE) ;
"!~"    return TOKEN(T_NOT_MATCHES) ;
"!"     return TOKEN(T_NOT) ;
"("     return TOKEN(T_LPAR) ;
")"     return TOKEN(T_RPAR) ;
"+"     return TOKEN(T_PLUS) ;
"-"     return TOKEN(T_MINUS) ;
"*"     return TOKEN(T_STAR) ;
"/"     return TOKEN(T_DIV) ;
","     return TOKEN(T_COMMA) ;
"{"		return TOKEN(T_LEFT_BRACE) ;
"}"		return TOKEN(T_RIGHT_BRACE) ;
"."     return TOKEN(T_PERIOD) ;
"]"		return TOKEN(T_RIGHT_BRACKET) ;
"["	    return TOKEN(T_LEFT_BRACKET) ;
":"	    return TOKEN(T_COLON) ;
"|"	    return TOKEN(T_BAR) ;
"?"	    return TOKEN(T_QUESTION_MARK) ;

[a-zA-Z][a-zA-Z0-9_]* {
    return yy::Parser::make_T_IDENTIFIER(yytext, *yylloc);
}

[0-9]+	  return yy::Parser::make_T_INTEGER(atof(yytext), *yylloc) ;

[0-9]+"."[0-9]*    |
"."[0-9]*	|
[0-9]+[eE][+-]?[0-9]+	|
[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
"."[0-9]*[eE][+-]?[0-9]+ {
    return yy::Parser::make_T_FLOAT(atof(yytext), *yylloc) ;
}

\"[^\"]*\" { return yy::Parser::make_T_STRING(std::string(yytext+1, yyleng-2), *yylloc) ; }
\'[^\']*\' { return yy::Parser::make_T_STRING(std::string(yytext+1, yyleng-2), *yylloc) ; }

"#"[^\n]*\n { yylloc->step();}

"/*" { BEGIN(COMMENT); }

}

<SUBS>{
"}}"    { BEGIN(INITIAL) ; return TOKEN(T_DOUBLE_RIGHT_BRACE) ; }
"-}}"    { BEGIN(INITIAL) ; return TOKEN(T_DOUBLE_RIGHT_BRACE_TRIM) ; }
}

[\n]+   yylloc->lines (yyleng); yylloc->step ();

[ \t]+   {yylloc->step ();}

<COMMENT>\n { yylloc->lines (); yylloc->step ();}
<COMMENT>"*/" { BEGIN(TAG) ; }
<COMMENT>.  { yylloc->step (); }

<TCOMMENT>\n { yylloc->lines (); yylloc->step ();}
<TCOMMENT>"#}" { BEGIN(INITIAL) ; }
<TCOMMENT>.  { yylloc->step (); }


"{{"    { BEGIN(SUBS) ; return TOKEN(T_DOUBLE_LEFT_BRACE) ; }
"{{-"   { BEGIN(SUBS) ; return TOKEN(T_DOUBLE_LEFT_BRACE_TRIM) ; }
"{%" { BEGIN(TAG); return TOKEN(T_START_BLOCK_TAG) ; }
"{%-" { BEGIN(TAG); return TOKEN(T_START_BLOCK_TAG_TRIM) ; }
"{#" { BEGIN(TCOMMENT) ; }
"{"|[^{]* {
/* this introduces a costly copy */
    return yy::Parser::make_T_RAW_CHARACTERS(std::string(yytext, yyleng), *yylloc) ;
}

<<EOF>>  return TOKEN(T_END);
%%


