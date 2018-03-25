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

%x COMMENT TAG SUBS

%{
# define YY_USER_ACTION  yylloc->columns (yyleng);
        using namespace std ;
%}
%%

%{
   yylloc->step();

%}

<TAG>{
  "block" return yy::Parser::make_BEGIN_BLOCK(*yylloc) ;
  "endblock" return yy::Parser::make_END_BLOCK(*yylloc) ;
  "for"     return yy::Parser::make_FOR(*yylloc) ;
  "endfor"  return yy::Parser::make_END_FOR(*yylloc) ;
  "else"    return yy::Parser::make_ELSE(*yylloc) ;
  "in"	    return yy::Parser::make_IN(*yylloc);
  "%}" { BEGIN(INITIAL) ;  return yy::Parser::make_END_BLOCK_TAG(*yylloc) ; }
}

<TAG,SUBS>{

"and"   return yy::Parser::make_AND(*yylloc);
"&&"    return yy::Parser::make_AND(*yylloc);
"or"    return yy::Parser::make_OR(*yylloc);
"||"    return yy::Parser::make_OR(*yylloc);
"not"	return yy::Parser::make_NOT(*yylloc);

"true"  return yy::Parser::make_TRUEX(*yylloc);
"false" return yy::Parser::make_FALSEX(*yylloc);
"null"  return yy::Parser::make_NULLX(*yylloc);

"="     return yy::Parser::make_ASSIGN(*yylloc) ;
"=="    return yy::Parser::make_EQUAL(*yylloc) ;
"<>"    return yy::Parser::make_NOT_EQUAL(*yylloc) ;
"!="    return yy::Parser::make_NOT_EQUAL(*yylloc) ;
"<"     return yy::Parser::make_LESS_THAN(*yylloc) ;
">"     return yy::Parser::make_GREATER_THAN(*yylloc) ;
"<="	return yy::Parser::make_LESS_THAN_OR_EQUAL(*yylloc) ;
">="	return yy::Parser::make_GREATER_THAN_OR_EQUAL(*yylloc) ;
"~"     return yy::Parser::make_TILDE(*yylloc) ;
"!~"    return yy::Parser::make_NOT_MATCHES(*yylloc) ;
"!"     return yy::Parser::make_NOT(*yylloc) ;
"^"		return yy::Parser::make_EXISTS(*yylloc) ;
"("     return yy::Parser::make_LPAR(*yylloc) ;
")"     return yy::Parser::make_RPAR(*yylloc) ;
"+"     return yy::Parser::make_PLUS(*yylloc) ;
"-"     return yy::Parser::make_MINUS(*yylloc) ;
"*"     return yy::Parser::make_STAR(*yylloc) ;
"/"     return yy::Parser::make_DIV(*yylloc) ;
","     return yy::Parser::make_COMMA(*yylloc) ;
"{"		return yy::Parser::make_LEFT_BRACE(*yylloc) ;
"}"		return yy::Parser::make_RIGHT_BRACE(*yylloc) ;
"."     return yy::Parser::make_PERIOD(*yylloc) ;
"]"		return yy::Parser::make_RIGHT_BRACKET(*yylloc) ;
"["	    return yy::Parser::make_LEFT_BRACKET(*yylloc) ;
":"	    return yy::Parser::make_COLON(*yylloc) ;
"|"	    return yy::Parser::make_BAR(*yylloc) ;


[a-zA-Z][a-zA-Z0-9_]* {
            return yy::Parser::make_IDENTIFIER(yytext, *yylloc);
}

[+-]?[0-9]+	  return yy::Parser::make_INTEGER(atof(yytext), *yylloc) ;

[+-]?[0-9]+"."[0-9]*    |
[+-]?"."[0-9]*	|
[+-]?[0-9]+[eE][+-]?[0-9]+	|
[+-]?[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
[+-]?"."[0-9]*[eE][+-]?[0-9]+ {
        return yy::Parser::make_FLOAT(atof(yytext), *yylloc) ;
}

\"[^\"]*\" { return yy::Parser::make_STRING(std::string(yytext+1, yyleng-2), *yylloc) ; }
\'[^\']*\' { return yy::Parser::make_STRING(std::string(yytext+1, yyleng-2), *yylloc) ; }

"#"[^\n]*\n { yylloc->step();}

"/*" { BEGIN(COMMENT); }

}

<SUBS>{
"}}"    { BEGIN(INITIAL) ; return yy::Parser::make_DOUBLE_RIGHT_BRACE(*yylloc) ; }
}

[\n]+   yylloc->lines (yyleng); yylloc->step ();

[ \t]+   {yylloc->step ();}

<COMMENT>\n { yylloc->lines (); yylloc->step ();}
<COMMENT>"*/" { BEGIN(TAG) ; }
<COMMENT>.  { yylloc->step (); }


"{{"    { BEGIN(SUBS) ; return yy::Parser::make_DOUBLE_LEFT_BRACE(*yylloc) ; }
"{%" { BEGIN(TAG); return yy::Parser::make_START_BLOCK_TAG(*yylloc) ; }
"{"|[^{]* {
           return yy::Parser::make_RAW_CHARACTERS(std::string(yytext, yyleng), *yylloc) ;
}

<<EOF>>    return yy::Parser::make_END(*yylloc);
%%


