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
%token  NOT_MATCHES "!~"
%token EQUAL "=="
%token NOT_EQUAL "!="
%token LESS_THAN "<"
%token GREATER_THAN ">"
%token LESS_THAN_OR_EQUAL "<="
%token GREATER_THAN_OR_EQUAL ">="
%token TRUEX "true"
%token FALSEX "false"
%token NULLX "null"
%token EXISTS "^"
%token PLUS "+"
%token MINUS "-"
%token STAR "*"
%token DIV "/"
%token LPAR "("
%token RPAR ")"
%token COMMA ","
%token PERIOD "."
%token COLON ":"
%token LEFT_BRACE "{"
%token RIGHT_BRACE "}"
%token LEFT_BRACKET "["
%token RIGHT_BRACKET "]"

%token FOR "for"
%token END_FOR "endfor"
%token ELSE "else"
%token SET_CMD "set tag"
%token DELETE_CMD "delete tag"
%token WRITE_CMD "write"
%token CONTINUE_CMD "continue"
%token ASSIGN "="
%token IN "in"
%token START_BLOCK_TAG "<%"
%token END_BLOCK_TAG "%>"
%token RAW_CHARACTERS "raw characters";
%token BEGIN_BLOCK "block"
%token END_BLOCK "endblock"
%token DOUBLE_LEFT_BRACE "{{"
%token DOUBLE_RIGHT_BRACE "}}"
%token TILDE "~"
%token BAR "|"

%token <std::string> IDENTIFIER "identifier";
%token <std::string> RAW_CHARACTERS "identifier";
%token <int64_t> INTEGER "integer";
%token <double> FLOAT "float";
%token <std::string> STRING "string literal";

%token END  0  "end of file";

%type <ast::ExpressionNodePtr> boolean_value_expression boolean_term boolean_factor boolean_primary predicate comparison_predicate
%type <ast::ExpressionNodePtr> expression term factor primary_expression value array object
%type <ast::ExpressionNodePtr> unary_predicate
%type <ast::ExpressionListPtr> expression_list filter_argument_list
%type <ast::KeyValListPtr> key_val_list
%type <ast::KeyValNodePtr> key_val
%type <ast::FilterNodePtr> filter
%type <ast::IdentifierListPtr> identifier_list
%type <ast::ContentNodePtr> for_loop_declaration block_declaration block_tag sub_tag tag_or_chars

/*operators */

%left OR
%left AND
%left LESS_THAN GREATER_THAN LESS_THAN_OR_EQUAL GREATER_THAN_OR_EQUAL EQUAL NOT_EQUAL
%left PLUS MINUS TILDE
%left STAR DIV
%nonassoc UMINUS EXISTS

%start document

%%

document:
|
mixed_tag_chars_list

mixed_tag_chars_list:
    tag_or_chars
   | tag_or_chars mixed_tag_chars_list


tag_or_chars:
    block_tag        { $$ = $1 ; }
    | sub_tag        { $$ = $1 ; }
    | RAW_CHARACTERS {
    driver.addNode(std::make_shared<ast::RawTextNode>($1)) ;
    }

block_tag:
    START_BLOCK_TAG tag_declaration END_BLOCK_TAG ;

sub_tag:
    DOUBLE_LEFT_BRACE expression DOUBLE_RIGHT_BRACE { driver.root_ = $2 ; }

tag_declaration:
    block_declaration
   | endblock_declaration
   | for_loop_declaration
   | else_declaration
   | end_for_declaration

block_declaration:
    BEGIN_BLOCK IDENTIFIER

endblock_declaration:
    END_BLOCK

for_loop_declaration:
    FOR identifier_list IN expression {
        auto node = std::make_shared<ast::ForLoopBlockNode>($2, $4) ;
        driver.addNode(node) ;
        driver.pushBlock(node);
    }

end_for_declaration:
    END_FOR { driver.popBlock() ; }

else_declaration:
    ELSE

identifier_list:
    IDENTIFIER                          { $$ = std::make_shared<ast::IdentifierList>() ; $$->append($1) ; }
    | IDENTIFIER COMMA identifier_list  { $$ = $3 ; $3->prepend($1) ; }


boolean_value_expression:
        boolean_term					{ $$ = $1 ; }
        | boolean_value_expression OR boolean_term 	{ $$ = std::make_shared<ast::BooleanOperator>( ast::BooleanOperator::Or, $1, $3) ; }
	;

boolean_term:
        boolean_factor				{ $$ = $1 ; }
        | boolean_term AND boolean_factor	{ $$ = std::make_shared<ast::BooleanOperator>( ast::BooleanOperator::And, $1, $3) ; }
	;

boolean_factor:
        boolean_primary		{ $$ = $1 ; }
        | NOT boolean_primary	{ $$ = std::make_shared<ast::BooleanOperator>( ast::BooleanOperator::Not, $2, nullptr) ; }
	;

boolean_primary:
        predicate				{ $$ = $1 ; }
	| LPAR boolean_value_expression RPAR	{ $$ = $2 ; }
	;

predicate:
	unary_predicate { $$ = $1 ; }
	|  comparison_predicate	{ $$ = $1 ; }
/*	| like_text_predicate	{ $$ = $1 ; }
        | list_predicate        { $$ = $1 ; }
        */
	;

unary_predicate:
        expression { $$ = std::make_shared<ast::UnaryPredicate>( $1 ) ;}

comparison_predicate:
        expression EQUAL expression			{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::Equal, $1, $3 ) ; }
        | expression NOT_EQUAL expression		{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::NotEqual, $1, $3 ) ; }
        | expression LESS_THAN expression		{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::Less, $1, $3 ) ; }
        | expression GREATER_THAN expression		{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::Greater, $1, $3 ) ; }
        | expression LESS_THAN_OR_EQUAL expression	{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::LessOrEqual, $1, $3 ) ; }
        | expression GREATER_THAN_OR_EQUAL expression	{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::GreaterOrEqual, $1, $3 ) ; }
	 ;


expression:
                  term			{ $$ = $1 ; }
                | term PLUS expression	{ $$ = std::make_shared<ast::BinaryOperator>('+',$1, $3) ; }
                | term TILDE expression	{ $$ = std::make_shared<ast::BinaryOperator>('~',$1, $3) ; }
                | term MINUS expression	{ $$ = std::make_shared<ast::BinaryOperator>('-',$1, $3) ; }
	  ;

term:
                factor				{ $$ = $1 ; }
                | factor STAR term		{ $$ = std::make_shared<ast::BinaryOperator>('*', $1, $3) ; }
                | factor DIV term		{ $$ = std::make_shared<ast::BinaryOperator>('/', $1, $3) ; }
		;

primary_expression
                        : IDENTIFIER            { $$ = std::make_shared<ast::IdentifierNode>($1) ; }
                        | value                 { $$ = std::make_shared<ast::ValueNode>($1) ; }
                        |  LPAR expression RPAR { $$ = $2 ; }

factor
        : primary_expression                                { $$ = $1 ; }
        | factor  LEFT_BRACKET expression RIGHT_BRACKET     { $$ = std::make_shared<ast::SubscriptIndexingNode>($1, $3) ; }
        | factor BAR filter                                 { $$ = std::make_shared<ast::ApplyFilterNode>($1, $3) ; }
        | factor PERIOD IDENTIFIER                          { $$ = std::make_shared<ast::AttributeIndexingNode>($1, $3) ; }


filter:
                IDENTIFIER	{ $$ = std::make_shared<ast::FilterNode>($1) ; }
                 | IDENTIFIER LPAR filter_argument_list RPAR {
                        $$ = std::make_shared<ast::FilterNode>($1, $3) ;
		 }
	;

filter_argument_list:
    expression_list { $$ = $1 ; }

value:
    STRING          { $$ = std::make_shared<ast::LiteralNode>($1) ; }
    | INTEGER        { $$ = std::make_shared<ast::LiteralNode>($1) ; }
    | FLOAT        { $$ = std::make_shared<ast::LiteralNode>($1) ; }
    | object        { $$ = $1 ; }
    | array         { $$ = $1 ; }
    | TRUEX         { $$ = std::make_shared<ast::LiteralNode>(true) ; }
    | FALSEX        { $$ = std::make_shared<ast::LiteralNode>(false) ; }
    | NULLX         { $$ = std::make_shared<ast::LiteralNode>(nullptr) ; }

array:
    LEFT_BRACKET expression_list RIGHT_BRACKET { $$ = std::make_shared<ast::ArrayNode>($2) ; }

object:
    LEFT_BRACE key_val_list RIGHT_BRACE { $$ = std::make_shared<ast::DictionaryNode>($2) ; }

expression_list:
    expression                          {  $$ = std::make_shared<ast::ExpressionList>() ;
                                           $$->append($1) ;
                                        }
    | expression COMMA expression_list  {  $$ = $3 ;
    $3->prepend($1) ;
                                        }

key_val_list:
    key_val                             {   $$ = std::make_shared<ast::KeyValList>() ;
                                            $$->append($1) ;
                                        }
    | key_val COMMA key_val_list        {  $$ = $3 ; $3->prepend($1) ;
                                        }

key_val:
    STRING COLON expression { $$ = std::make_shared<ast::KeyValNode>($1, $3) ; }
    | IDENTIFIER COLON expression { $$ = std::make_shared<ast::KeyValNode>($1, $3) ; }

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


