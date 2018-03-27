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

using namespace ast ;

static yy::Parser::symbol_type yylex(TemplateParser &driver, yy::Parser::location_type &loc);
}

/* literal keyword tokens */
%token T_NOT "!"
%token T_AND "&&"
%token T_OR "||"
%token T_NOT_MATCHES "!~"
%token T_EQUAL "=="
%token T_NOT_EQUAL "!="
%token T_LESS_THAN "<"
%token T_GREATER_THAN ">"
%token T_LESS_THAN_OR_EQUAL "<="
%token T_GREATER_THAN_OR_EQUAL ">="
%token T_TRUE "true"
%token T_FALSE "false"
%token T_NULL "null"
%token T_QUESTION_MARK "?"
%token T_PLUS "+"
%token T_MINUS "-"
%token T_STAR "*"
%token T_DIV "/"
%token T_LPAR "("
%token T_RPAR ")"
%token T_COMMA ","
%token T_PERIOD "."
%token T_COLON ":"
%token T_LEFT_BRACE "{"
%token T_RIGHT_BRACE "}"
%token T_LEFT_BRACKET "["
%token T_RIGHT_BRACKET "]"
%token T_TILDE "~"
%token T_BAR "|"

%token T_FOR "for"
%token T_END_FOR "endfor"
%token T_ELSE "else"
%token T_ELSE_IF "elif"
%token T_END_IF "endif"
%token T_IF "if"
%token T_SET "set"
%token T_END_SET "endset"
%token T_FILTER "filter"
%token T_END_FILTER "endfilter"
%token T_ASSIGN "="
%token T_IN "in"
%token T_START_BLOCK_TAG "{%"
%token T_END_BLOCK_TAG "%}"
%token T_START_BLOCK_TAG_TRIM "{%-"
%token T_END_BLOCK_TAG_TRIM "-%}"
%token T_BEGIN_BLOCK "block"
%token T_END_BLOCK "endblock"
%token T_DOUBLE_LEFT_BRACE "{{"
%token T_DOUBLE_RIGHT_BRACE "}}"
%token T_DOUBLE_LEFT_BRACE_TRIM "{{-"
%token T_DOUBLE_RIGHT_BRACE_TRIM "-}}"

%token <std::string> T_IDENTIFIER "identifier";
%token <std::string> T_RAW_CHARACTERS "raw characters";
%token <int64_t> T_INTEGER "integer";
%token <double> T_FLOAT "float";
%token <std::string> T_STRING "string literal";

%token T_END  0  "end of file";

%type <ast::ExpressionNodePtr> expression value array object function_call
%type <ast::ExpressionListPtr> expression_list pos_func_args
%type <ast::KeyValListPtr> key_val_list
%type <ast::KeyValNodePtr> key_val
%type <ast::FilterNodePtr> filter function
%type <ast::FunctionArgumentsPtr> func_args
%type <ast::FunctionArgPtr> func_arg
%type <ast::IdentifierListPtr> identifier_list
%type <ast::ContentNodePtr> for_loop_declaration block_declaration block_tag sub_tag tag_or_chars

/*operators */


%right T_QUESTION_MARK T_COLON
%right T_ASSIGN
%left T_COMMA
%left T_OR
%left T_AND
%nonassoc T_LESS_THAN T_GREATER_THAN T_LESS_THAN_OR_EQUAL T_GREATER_THAN_OR_EQUAL T_EQUAL T_NOT_EQUAL
%left T_PLUS T_MINUS T_TILDE
%left T_STAR T_DIV
%right T_NOT
%right T_LEFT_BRACKET T_PERIOD

%start document

%%

document:
%empty
| mixed_tag_chars_list

mixed_tag_chars_list:
    tag_or_chars
   | tag_or_chars mixed_tag_chars_list


tag_or_chars:
    block_tag
    | sub_tag
    | T_RAW_CHARACTERS {
        driver.addNode(std::make_shared<ast::RawTextNode>($1)) ;
    }

block_tag:
T_START_BLOCK_TAG tag_declaration T_END_BLOCK_TAG
;

sub_tag:
    T_DOUBLE_LEFT_BRACE expression T_DOUBLE_RIGHT_BRACE {
        driver.addNode(std::make_shared<ast::SubTextNode>($2)) ;
    }
    | T_DOUBLE_LEFT_BRACE_TRIM expression T_DOUBLE_RIGHT_BRACE {
        driver.addNode(std::make_shared<ast::SubTextNode>($2, WhiteSpace::TrimLeft)) ;
    }
    | T_DOUBLE_LEFT_BRACE expression T_DOUBLE_RIGHT_BRACE_TRIM {
        driver.addNode(std::make_shared<ast::SubTextNode>($2, WhiteSpace::TrimRight)) ;
    }
    | T_DOUBLE_LEFT_BRACE_TRIM expression T_DOUBLE_RIGHT_BRACE_TRIM {
        driver.addNode(std::make_shared<ast::SubTextNode>($2, WhiteSpace::TrimBoth)) ;
    }


tag_declaration:
    block_declaration
   | endblock_declaration
   | for_loop_declaration
   | else_declaration
   | end_for_declaration
   | if_declaration
   | else_if_declaration
   | end_if_declaration
   | set_declaration
   | end_set_declaration
   | filter_declaration
   | end_filter_declaration
  ;

block_declaration:
    T_BEGIN_BLOCK T_IDENTIFIER
    ;

endblock_declaration:
    T_END_BLOCK
    ;

for_loop_declaration:
    T_FOR identifier_list T_IN expression {
        auto node = std::make_shared<ast::ForLoopBlockNode>($2, $4) ;
        driver.addNode(node) ;
        driver.pushBlock(node);
    }

end_for_declaration:
    T_END_FOR { driver.popBlock() ; }

else_declaration:
    T_ELSE {
        if ( ast::ForLoopBlockNode *p = dynamic_cast<ast::ForLoopBlockNode *>(driver.stack_.back().get()) )
            p->startElseBlock() ;
        else if ( ast::IfBlockNode *p = dynamic_cast<ast::IfBlockNode *>(driver.stack_.back().get()) )
            p->addBlock(nullptr) ;
    }

if_declaration:
   T_IF expression {
        auto node = std::make_shared<ast::IfBlockNode>($2) ;
        driver.addNode(node) ;
        driver.pushBlock(node);
   }

else_if_declaration:
    T_ELSE_IF expression {
        if ( ast::IfBlockNode *p = dynamic_cast<ast::IfBlockNode *>(driver.stack_.back().get()) )
            p->addBlock($2) ;
    }

end_if_declaration:
    T_END_IF { driver.popBlock() ; }

set_declaration:
    T_SET T_IDENTIFIER T_ASSIGN expression  {
        auto node = std::make_shared<ast::AssignmentBlockNode>($2, $4) ;
        driver.addNode(node) ;
        driver.pushBlock(node);
   }

end_set_declaration:
    T_END_SET { driver.popBlock() ; }

filter_declaration:
    T_FILTER filter  {
        auto node = std::make_shared<ast::FilterBlockNode>($2) ;
            driver.addNode(node) ;
            driver.pushBlock(node);
    }

end_filter_declaration:
        T_END_FILTER { driver.popBlock() ; }

identifier_list:
    T_IDENTIFIER                          { $$ = std::make_shared<ast::IdentifierList>() ; $$->append($1) ; }
    | T_IDENTIFIER T_COMMA identifier_list  { $$ = $3 ; $3->prepend($1) ; }


expression:
          expression T_OR expression      { $$ = std::make_shared<ast::BooleanOperator>( ast::BooleanOperator::Or, $1, $3) ; }
        | expression T_AND expression     { $$ = std::make_shared<ast::BooleanOperator>( ast::BooleanOperator::And, $1, $3) ; }
        | expression T_EQUAL expression			{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::Equal, $1, $3 ) ; }
        | expression T_NOT_EQUAL expression		{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::NotEqual, $1, $3 ) ; }
        | expression T_LESS_THAN expression		{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::Less, $1, $3 ) ; }
        | expression T_GREATER_THAN expression		{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::Greater, $1, $3 ) ; }
        | expression T_LESS_THAN_OR_EQUAL expression	{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::LessOrEqual, $1, $3 ) ; }
        | expression T_GREATER_THAN_OR_EQUAL expression	{ $$ = std::make_shared<ast::ComparisonPredicate>( ast::ComparisonPredicate::GreaterOrEqual, $1, $3 ) ; }
        | expression T_PLUS expression		{ $$ = std::make_shared<ast::BinaryOperator>('+', $1, $3) ; }
        | expression T_MINUS expression		{ $$ = std::make_shared<ast::BinaryOperator>('-', $1, $3) ; }
        | expression T_TILDE expression	{ $$ = std::make_shared<ast::BinaryOperator>('~', $1, $3) ; }
        | expression T_STAR expression		{ $$ = std::make_shared<ast::BinaryOperator>('*', $1, $3) ; }
        | expression T_DIV expression		{ $$ = std::make_shared<ast::BinaryOperator>('/', $1, $3) ; }
        | T_PLUS expression  { $$ = std::make_shared<ast::UnaryOperator>('+', $2) ; }
        | T_MINUS expression  { $$ = std::make_shared<ast::UnaryOperator>('-', $2) ; }
        | T_LPAR expression T_RPAR { $$ = $2; }
        | expression T_QUESTION_MARK expression T_COLON expression { $$ = std::make_shared<ast::TernaryExpressionNode>($1, $3, $5) ; }
        | expression  T_LEFT_BRACKET expression T_RIGHT_BRACKET     { $$ = std::make_shared<ast::SubscriptIndexingNode>($1, $3) ; }
        | expression T_BAR filter                                 { $$ = std::make_shared<ast::InvokeFilterNode>($1, $3) ; }
        | expression T_PERIOD T_IDENTIFIER                          { $$ = std::make_shared<ast::AttributeIndexingNode>($1, $3) ; }
        | function_call  { $$ = $1 ; }
        | value { $$ = $1 ; }
        | T_IDENTIFIER { $$ = std::make_shared<ast::IdentifierNode>($1) ; }

filter:
                T_IDENTIFIER	{
                    $$ = std::make_shared<ast::FilterNode>($1, nullptr) ;
                }
                | T_IDENTIFIER T_LPAR func_args T_RPAR    { $$ = std::make_shared<ast::FilterNode>($1, $3) ; }

function_call:
                 expression T_LPAR T_RPAR {
                        $$ = std::make_shared<ast::InvokeFunctionNode>($1, nullptr) ;
		 }
                 | expression T_LPAR func_args T_RPAR {
                        $$ = std::make_shared<ast::InvokeFunctionNode>($1, $3) ;
                 }
	;

func_args:
    func_arg { $$ = std::make_shared<ast::FunctionArguments>() ;
        $$->append($1) ;
    }
    | func_arg T_COMMA func_args { $$ = $3 ;  $3->prepend($1) ; }


func_arg:
    expression          { $$ = std::make_shared<ast::FunctionArg>($1) ; }
    | T_IDENTIFIER T_ASSIGN expression { $$ = std::make_shared<ast::FunctionArg>($3, $1) ; }

value:
    T_STRING          { $$ = std::make_shared<ast::LiteralNode>($1) ; }
    | T_INTEGER        { $$ = std::make_shared<ast::LiteralNode>($1) ; }
    | T_FLOAT        { $$ = std::make_shared<ast::LiteralNode>($1) ; }
    | object        { $$ = $1 ; }
    | array         { $$ = $1 ; }
    | T_TRUE         { $$ = std::make_shared<ast::LiteralNode>(true) ; }
    | T_FALSE        { $$ = std::make_shared<ast::LiteralNode>(false) ; }
    | T_NULL         { $$ = std::make_shared<ast::LiteralNode>(nullptr) ; }

array:
    T_LEFT_BRACKET expression_list T_RIGHT_BRACKET { $$ = std::make_shared<ast::ArrayNode>($2) ; }

object:
    T_LEFT_BRACE key_val_list T_RIGHT_BRACE { $$ = std::make_shared<ast::DictionaryNode>($2) ; }

expression_list:
    expression {  $$ = std::make_shared<ast::ExpressionList>() ;
                   $$->append($1) ;
                }
    | expression T_COMMA expression_list  {  $$ = $3 ;  $3->prepend($1) ; }

key_val_list:
    key_val                             {   $$ = std::make_shared<ast::KeyValList>() ;
                                            $$->append($1) ;
                                        }
    | key_val T_COMMA key_val_list        {  $$ = $3 ; $3->prepend($1) ;
                                        }

key_val:
    T_STRING T_COLON expression { $$ = std::make_shared<ast::KeyValNode>($1, $3) ; }
    | T_IDENTIFIER T_COLON expression { $$ = std::make_shared<ast::KeyValNode>($1, $3) ; }


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


