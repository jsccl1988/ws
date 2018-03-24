%require "3.0.2"
%skeleton "lalr1.cc"

%defines
%locations

%define parser_class_name {Parser}
%define api.token.constructor
%define api.value.type variant
%define api.token.prefix {TOK_}

%param { TemplateParser &driver }
%param { Parser::location_type &loc }

%define parse.trace
%define parse.error verbose

%code requires {
#include <template_ast.hpp>
class TemplateParser ;

}

%code {
#include <parser.hpp>
#include <template_ast.hpp>
static yy::Parser::symbol_type yylex(TemplateParser &driver, yy::Parser::location_type &loc);
}

/* literal keyword tokens */
%token NOT "!"
%token AND "&&"
%token OR "||"
%token MATCHES "~"
%token  NOT_MATCHES "!~"
%token EQUAL "=="
%token NOT_EQUAL "!="
%token LESS_THAN "<"
%token GREATER_THAN ">"
%token LESS_THAN_OR_EQUAL "<="
%token GREATER_THAN_OR_EQUAL ">="
%token TRUEX "true"
%token FALSEX "false"
%token EXISTS "^"
%token PLUS "+"
%token MINUS "-"
%token STAR "*"
%token DIV "/"
%token LPAR "("
%token RPAR ")"
%token COMMA ","
%token DOT "."
%token COLON ":"
%token LEFT_BRACE "{"
%token RIGHT_BRACE "}"
%token LEFT_BRACKET "["
%token RIGHT_BRACKET "]"

%token ADD_CMD "add tag"
%token SET_CMD "set tag"
%token DELETE_CMD "delete tag"
%token WRITE_CMD "write"
%token CONTINUE_CMD "continue"
%token ASSIGN "="
%token IN "in"
%token START_TAG "<%"
%token END_TAG "%>"

%token <std::string> IDENTIFIER "identifier";
%token <double> NUMBER "number";
%token <std::string> STRING "string literal";
%token <std::string> LUA_SCRIPT "LUA script";
%token <uint8_t> ZOOM_SPEC "zoom specifier"
%token END  0  "end of file";

%type <ast::ExpressionNodePtr> boolean_value_expression boolean_term boolean_factor boolean_primary predicate comparison_predicate like_text_predicate exists_predicate
%type <ast::ExpressionNodePtr> expression term factor numeric_literal boolean_literal general_literal literal function function_argument attribute
%type <ast::ExpressionNodePtr> list_predicate unary_predicate
%type <ast::ExpressionListPtr> literal_list function_argument_list
//%type <ast::CommandPtr> command
//%type <ast::CommandListPtr> command_list action_block
//%type <ast::RulePtr> rule
//%type <ast::RuleListPtr> rule_list
//%type <ast::ZoomRangePtr> zoom_range
//%type <ast::TagListPtr> tag_list
//%type <ast::TagDeclarationPtr> tag_decl
//%type <ast::TagDeclarationListPtr> tag_decl_list

/*operators */

%left OR
%left AND
%left LESS_THAN GREATER_THAN LESS_THAN_OR_EQUAL GREATER_THAN_OR_EQUAL EQUAL NOT_EQUAL
%left PLUS MINUS DOT
%left STAR DIV
%nonassoc UMINUS EXISTS

%start document

%%

document:
| declaration_list

declaration_list:
        declaration
        | declaration declaration_list

declaration: START_TAG boolean_value_expression END_TAG ;

boolean_value_expression:
	boolean_term								{ $$ = $1 ; }
        | boolean_value_expression OR boolean_term 	{ $$ = std::make_shared<ast::BooleanOperator>( ast::BooleanOperator::Or, $1, $3) ; }
	;

boolean_term:
	boolean_factor						{ $$ = $1 ; }
        | boolean_term AND boolean_factor	{ $$ = std::make_shared<ast::BooleanOperator>( ast::BooleanOperator::And, $1, $3) ; }
	;

boolean_factor:
	boolean_primary			{ $$ = $1 ; }
        | NOT boolean_primary	{ $$ = std::make_shared<ast::BooleanOperator>( ast::BooleanOperator::Not, $2, nullptr) ; }
	;

boolean_primary:
	predicate							{ $$ = $1 ; }
	| LPAR boolean_value_expression RPAR	{ $$ = $2 ; }
	;

predicate:
	unary_predicate { $$ = $1 ; }
	|  comparison_predicate	{ $$ = $1 ; }
	| like_text_predicate	{ $$ = $1 ; }
	| list_predicate        { $$ = $1 ; }
	;

unary_predicate:
        expression { $$ = std::make_shared<ast::UnaryPredicate>( $1 ) ;}

comparison_predicate:
        expression EQUAL expression					{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::Equal, $1, $3 ) ; }
        | expression NOT_EQUAL expression				{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::NotEqual, $1, $3 ) ; }
        | expression LESS_THAN expression				{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::Less, $1, $3 ) ; }
        | expression GREATER_THAN expression			{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::Greater, $1, $3 ) ; }
        | expression LESS_THAN_OR_EQUAL expression		{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::LessOrEqual, $1, $3 ) ; }
        | expression GREATER_THAN_OR_EQUAL expression	{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::GreaterOrEqual, $1, $3 ) ; }
	 ;

like_text_predicate:
        expression MATCHES STRING						{ $$ = std::make_shared<ast::LikeTextPredicate>($1, $3, true) ; }
        | expression NOT_MATCHES STRING					{ $$ = std::make_shared<ast::LikeTextPredicate>($1, $3, false) ; }
	;

list_predicate:
        IDENTIFIER IN LPAR literal_list RPAR		{ $$ = std::make_shared<ast::ListPredicate>($1, $4->children(), true) ; }
        | IDENTIFIER NOT IN LPAR literal_list RPAR	{ $$ = std::make_shared<ast::ListPredicate>($1, $5->children(), false) ; }
	;

literal_list:
        literal		{ $$ = std::make_shared<ast::ExpressionList>() ;  $$->append($1) ;  }
	| literal COMMA literal_list { $$ = $3 ; $3->prepend($1) ; }
	;

expression:
		  term					{ $$ = $1 ; }
                | term PLUS expression	{ $$ = std::make_shared<ast::BinaryOperator>('+',$1, $3) ; }
                | term DOT expression	{ $$ = std::make_shared<ast::BinaryOperator>('.',$1, $3) ; }
                | term MINUS expression	{ $$ = std::make_shared<ast::BinaryOperator>('-', $1, $3) ; }
	  ;

term:
		factor					{ $$ = $1 ; }
                | factor STAR term		{ $$ = std::make_shared<ast::BinaryOperator>('*', $1, $3) ; }
                | factor DIV term		{ $$ = std::make_shared<ast::BinaryOperator>('/', $1, $3) ; }
		;

factor:
		  function				{ $$ = $1 ; }
		| literal				{ $$ = $1 ; }
		| attribute			{ $$ = $1 ; }
		| LPAR expression RPAR	{ $$ = $2 ; }
		;

function:
                IDENTIFIER LPAR RPAR		{ $$ = std::make_shared<ast::Function>($1) ; }
		 | IDENTIFIER LPAR function_argument_list RPAR {
                        $$ = std::make_shared<ast::Function>($1, $3->children()) ;
		 }
	;

function_argument_list:
		  function_argument		{
                                $$ = std::make_shared<ast::ExpressionList>() ;
				$$->append($1) ;
			}
		| function_argument COMMA function_argument_list { $$ = $3 ; $3->prepend($1) ; }
		;

function_argument :
		expression			{ $$ = $1 ; }
		;

literal:
		numeric_literal		{ $$ = $1 ; }
		| general_literal	{ $$ = $1 ; }
		;

general_literal :
                STRING				{ $$ = std::make_shared<ast::LiteralExpressionNode>($1) ; }
		| boolean_literal	{ $$ = $1 ; }

		;

boolean_literal:
                TRUEX	{ $$ = std::make_shared<ast::LiteralExpressionNode>(true) ; }
                | FALSEX { $$ =  std::make_shared<ast::LiteralExpressionNode>(false) ; }
	;

numeric_literal:
	NUMBER {
                $$ = std::make_shared<ast::LiteralExpressionNode>((double)$1) ;
	}
	;

attribute:
	IDENTIFIER {
                $$ = std::make_shared<ast::Attribute>($1) ;
	}
	;

%%
#define YYDEBUG 1

#include "scanner.hpp"

// We have to implement the error function
void yy::Parser::error(const yy::Parser::location_type &loc, const std::string &msg) {
	driver.error(loc, msg) ;
}

// Now that we have the Parser declared, we can declare the Scanner and implement
// the yylex function

static yy::Parser::symbol_type yylex(TemplateParser &driver, yy::Parser::location_type &loc) {
	return  driver.scanner_.lex(&loc);
}


