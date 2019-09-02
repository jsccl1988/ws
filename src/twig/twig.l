%{
#include "parser.hpp"

#include "bison_parser.hpp"

#include <wspp/twig/functions.hpp>

#include <boost/algorithm/string.hpp>

#include <string.h>

void comment();
%}

%option debug noyywrap
%option yyclass="TemplateScanner"
%option c++
%option debug
%option case-insensitive

%x COMMENT TAG SUBS TCOMMENT VERBATIM

%{
# define YY_USER_ACTION  yylloc->columns (yyleng);
using namespace std;

#define TOKEN(name) yy::Parser::make_##name(*yylloc)

%}
%%

%{
   yylloc->step();

%}

<TAG>{
  "block"       return TOKEN(T_BEGIN_BLOCK);
  "endblock"    return TOKEN(T_END_BLOCK);
  "for"         return TOKEN(T_FOR);
  "endfor"      return TOKEN(T_END_FOR);
  "elif"        return TOKEN(T_ELSE_IF);
  "endif"       return TOKEN(T_END_IF);
  "set"         return TOKEN(T_SET);
  "endset"      return TOKEN(T_END_SET);
  "filter"      return TOKEN(T_FILTER);
  "endfilter"   return TOKEN(T_END_FILTER);
  "extends"     return TOKEN(T_EXTENDS);
  "endextends"  return TOKEN(T_END_EXTENDS);
  "embed"       return TOKEN(T_EMBED);
  "endembed"    return TOKEN(T_END_EMBED);
  "macro"       return TOKEN(T_MACRO);
  "endmacro"    return TOKEN(T_END_MACRO);
  "import"      return TOKEN(T_IMPORT);
  "include"     return TOKEN(T_INCLUDE);
  "ignore"      return TOKEN(T_IGNORE);
  "missing"     return TOKEN(T_MISSING);
  "with"        return TOKEN(T_WITH);
  "endwith"     return TOKEN(T_END_WITH);
  "only"        return TOKEN(T_ONLY);
  "from"        return TOKEN(T_FROM);
  "as"          return TOKEN(T_AS);
  "_self"       return TOKEN(T_SELF);
  "in"          return TOKEN(T_IN);
  "is"          return TOKEN(T_IS);
  "matches"     return TOKEN(T_MATCHES);

  "autoescape"  return TOKEN(T_AUTOESCAPE);
  "endautoescape"  return TOKEN(T_END_AUTOESCAPE);
  "%}"          { BEGIN(INITIAL);  return TOKEN(T_END_BLOCK_TAG); }
  "-%}"         { BEGIN(INITIAL);  return TOKEN(T_END_BLOCK_TAG_TRIM); }
}

<VERBATIM>{
"{%"[ ]*"endverbatim"[ ]*"%}"  { BEGIN(INITIAL); }
}

<TAG,SUBS>{

"if"    return TOKEN(T_IF);
"else"  return TOKEN(T_ELSE);
"and"   return TOKEN(T_AND);
"&&"    return TOKEN(T_AND);
"or"    return TOKEN(T_OR);
"||"    return TOKEN(T_OR);
"not"	return TOKEN(T_NOT);

"true"  return TOKEN(T_TRUE);
"false" return TOKEN(T_FALSE);
"null"  return TOKEN(T_NULL);
"context"     return TOKEN(T_CONTEXT);

"="     return TOKEN(T_ASSIGN);
"=="    return TOKEN(T_EQUAL);
"<>"    return TOKEN(T_NOT_EQUAL);
"!="    return TOKEN(T_NOT_EQUAL);
"<"     return TOKEN(T_LESS_THAN);
">"     return TOKEN(T_GREATER_THAN);
"<="	return TOKEN(T_LESS_THAN_OR_EQUAL);
">="	return TOKEN(T_GREATER_THAN_OR_EQUAL);
"~"     return TOKEN(T_TILDE);
"!~"    return TOKEN(T_NOT_MATCHES);
"!"     return TOKEN(T_NOT);
"%"     return TOKEN(T_MOD);
"("     return TOKEN(T_LPAR);
")"     return TOKEN(T_RPAR);
"+"     return TOKEN(T_PLUS);
"-"     return TOKEN(T_MINUS);
"*"     return TOKEN(T_STAR);
"/"     return TOKEN(T_DIV);
","     return TOKEN(T_COMMA);
"{"		return TOKEN(T_LEFT_BRACE);
"}"		return TOKEN(T_RIGHT_BRACE);
"."     return TOKEN(T_PERIOD);
"]"		return TOKEN(T_RIGHT_BRACKET);
"["	    return TOKEN(T_LEFT_BRACKET);
":"	    return TOKEN(T_COLON);
"|"	    return TOKEN(T_BAR);
"?"	    return TOKEN(T_QUESTION_MARK);

"in"          return TOKEN(T_IN);
"is"          return TOKEN(T_IS);
"matches"     return TOKEN(T_MATCHES);

[a-zA-Z_][a-zA-Z0-9_]* {
   return yy::Parser::make_T_IDENTIFIER(yytext, *yylloc);
}

[0-9]+	  return yy::Parser::make_T_INTEGER(atof(yytext), *yylloc);

[0-9]+"."[0-9]*    |
"."[0-9]*	|
[0-9]+[eE][+-]?[0-9]+	|
[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
"."[0-9]*[eE][+-]?[0-9]+ {
    return yy::Parser::make_T_FLOAT(atof(yytext), *yylloc);
}

\"[^\"]*\" { return yy::Parser::make_T_STRING(std::string(yytext+1, yyleng-2), *yylloc); }
\'[^\']*\' { return yy::Parser::make_T_STRING(std::string(yytext+1, yyleng-2), *yylloc); }

"#"[^\n]*\n { yylloc->step();}

"/*" { BEGIN(COMMENT); }
}

<SUBS>{
"}}"    { BEGIN(INITIAL); return TOKEN(T_DOUBLE_RIGHT_BRACE); }
"-}}"    { BEGIN(INITIAL); return TOKEN(T_DOUBLE_RIGHT_BRACE_TRIM); }
}

<TAG,SUBS,TCOMMENT,COMMENT>{
[\n]+  {
    yylloc->lines (yyleng);
    yylloc->step ();
}

[ \r\t]+   {
    yylloc->step ();
}
}

<COMMENT>"*/" { BEGIN(TAG); }
<COMMENT>.  { yylloc->step (); }

<TCOMMENT>"#}" { BEGIN(INITIAL); }
<TCOMMENT>.  { yylloc->step (); }

"{%"[ ]*"verbatim"[ ]*"%}" { BEGIN(VERBATIM); }
"{{"    { BEGIN(SUBS); return TOKEN(T_DOUBLE_LEFT_BRACE); }
"{{-"   { BEGIN(SUBS); return TOKEN(T_DOUBLE_LEFT_BRACE_TRIM); }
"{%" { BEGIN(TAG); return TOKEN(T_START_BLOCK_TAG); }
"{%-" { BEGIN(TAG); trim_previous_raw_block_ = true; return TOKEN(T_START_BLOCK_TAG_TRIM); }
"{#" { BEGIN(TCOMMENT); }
<INITIAL,VERBATIM>"{"|[^{]+ {

    // we have to undo the default action
    yylloc->columns(-yyleng);

    string text;
    uint i = 0;
    for( char *c = yytext; i < yyleng; i++, c++ ) {
        if ( *c == '\n' ) {
            yylloc->lines();
            if ( i > 0 ) text.push_back(*c); // remove the first new line character
        }
        else {
            yylloc->columns();
            text.push_back(*c);
        }
    }

    if ( trim_next_raw_block_ ) {
        boost::trim_left(text);
        trim_next_raw_block_ = false;
    }

    if ( !text.empty() )
        return yy::Parser::make_T_RAW_CHARACTERS(text, *yylloc);
}

<<EOF>>  return TOKEN(T_END);
%%


