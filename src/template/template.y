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
using namespace std ;

static yy::Parser::symbol_type yylex(TemplateParser &driver, yy::Parser::location_type &loc);
}

/* literal keyword tokens */
%token T_NOT            "!"
%token T_AND            "&&"
%token T_OR             "||"
%token T_NOT_MATCHES    "!~"
%token T_EQUAL          "=="
%token T_NOT_EQUAL      "!="
%token T_LESS_THAN      "<"
%token T_GREATER_THAN   ">"
%token T_LESS_THAN_OR_EQUAL     "<="
%token T_GREATER_THAN_OR_EQUAL  ">="
%token T_TRUE           "true"
%token T_FALSE          "false"
%token T_NULL           "null"
%token T_QUESTION_MARK  "?"
%token T_PLUS           "+"
%token T_MINUS          "-"
%token T_STAR           "*"
%token T_DIV            "/"
%token T_LPAR           "("
%token T_RPAR           ")"
%token T_COMMA          ","
%token T_PERIOD         "."
%token T_COLON          ":"
%token T_LEFT_BRACE     "{"
%token T_RIGHT_BRACE    "}"
%token T_LEFT_BRACKET   "["
%token T_RIGHT_BRACKET  "]"
%token T_TILDE          "~"
%token T_BAR            "|"

%token T_FOR            "for"
%token T_END_FOR        "endfor"
%token T_ELSE           "else"
%token T_ELSE_IF        "elif"
%token T_END_IF         "endif"
%token T_IF             "if"
%token T_SET            "set"
%token T_END_SET        "endset"
%token T_FILTER         "filter"
%token T_END_FILTER     "endfilter"
%token T_EXTENDS        "extends"
%token T_MACRO          "macro"
%token T_END_MACRO      "end macro"
%token T_SELF           "_self"
%token T_AS             "as"
%token T_IMPORT         "import"
%token T_ASSIGN         "="
%token T_IN             "in"
%token T_BEGIN_BLOCK    "block"
%token T_END_BLOCK      "endblock"
%token T_START_BLOCK_TAG        "{%"
%token T_END_BLOCK_TAG          "%}"
%token T_START_BLOCK_TAG_TRIM   "{%-"
%token T_END_BLOCK_TAG_TRIM     "-%}"

%token T_DOUBLE_LEFT_BRACE      "{{"
%token T_DOUBLE_RIGHT_BRACE     "}}"
%token T_DOUBLE_LEFT_BRACE_TRIM "{{-"
%token T_DOUBLE_RIGHT_BRACE_TRIM "-}}"
%token T_END  0  "end of file";

%token <std::string> T_IDENTIFIER      "identifier";
%token <std::string> T_RAW_CHARACTERS  "raw characters";
%token <int64_t>     T_INTEGER         "integer";
%token <double>      T_FLOAT           "float";
%token <std::string> T_STRING          "string literal";


%type <ast::ExpressionNodePtr> expression value array object function_call
%type <ast::ExpressionListPtr> expression_list
%type <ast::KeyValListPtr> key_val_list
%type <ast::KeyValNodePtr> key_val
%type <ast::FilterNodePtr> filter
%type <ast::FunctionArgumentsPtr> func_args
%type <ast::FunctionArgPtr> func_arg
%type <ast::IdentifierListPtr> identifier_list
%type <ast::ContentNodePtr> block_tag sub_tag tag_or_chars tag_declaration
%type <ast::ContentNodePtr> block_declaration end_block_declaration for_loop_declaration end_for_declaration else_declaration if_declaration
%type <ast::ContentNodePtr> else_if_declaration end_if_declaration set_declaration end_set_declaration filter_declaration end_filter_declaration
%type <ast::ContentNodePtr> extends_declaration macro_declaration end_macro_declaration import_declaration

/*operators */

%right T_QUESTION_MARK T_COLON
%left T_OR
%left T_AND
%nonassoc T_LESS_THAN T_GREATER_THAN T_LESS_THAN_OR_EQUAL T_GREATER_THAN_OR_EQUAL T_EQUAL T_NOT_EQUAL
%left T_PLUS T_MINUS T_TILDE
%left T_STAR T_DIV
%right T_NOT
%right T_LEFT_BRACKET T_PERIOD
%right T_ASSIGN T_COMMA

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
        driver.addNode(make_shared<RawTextNode>($1)) ;
    }

block_tag:
T_START_BLOCK_TAG tag_declaration T_END_BLOCK_TAG { driver.stackTop()->setWhiteSpace(WhiteSpace::TrimNone) ; }
| T_START_BLOCK_TAG_TRIM tag_declaration T_END_BLOCK_TAG { driver.stackTop()->setWhiteSpace(WhiteSpace::TrimLeft) ; }
| T_START_BLOCK_TAG_TRIM tag_declaration T_END_BLOCK_TAG_TRIM { driver.stackTop()->setWhiteSpace(WhiteSpace::TrimBoth) ; }
| T_START_BLOCK_TAG tag_declaration T_END_BLOCK_TAG_TRIM { driver.stackTop()->setWhiteSpace(WhiteSpace::TrimRight) ; }

;

sub_tag:
    T_DOUBLE_LEFT_BRACE expression T_DOUBLE_RIGHT_BRACE {
        driver.addNode(make_shared<SubTextNode>($2)) ;
    }
    | T_DOUBLE_LEFT_BRACE_TRIM expression T_DOUBLE_RIGHT_BRACE {
        driver.addNode(make_shared<SubTextNode>($2, WhiteSpace::TrimLeft)) ;
    }
    | T_DOUBLE_LEFT_BRACE expression T_DOUBLE_RIGHT_BRACE_TRIM {
        driver.addNode(make_shared<SubTextNode>($2, WhiteSpace::TrimRight)) ;
    }
    | T_DOUBLE_LEFT_BRACE_TRIM expression T_DOUBLE_RIGHT_BRACE_TRIM {
        driver.addNode(make_shared<SubTextNode>($2, WhiteSpace::TrimBoth)) ;
    }


tag_declaration:
    block_declaration           { $$ = $1 ; }
   | end_block_declaration      { $$ = $1 ; }
   | for_loop_declaration       { $$ = $1 ; }
   | else_declaration           { $$ = $1 ; }
   | end_for_declaration        { $$ = $1 ; }
   | if_declaration             { $$ = $1 ; }
   | else_if_declaration        { $$ = $1 ; }
   | end_if_declaration         { $$ = $1 ; }
   | set_declaration            { $$ = $1 ; }
   | end_set_declaration        { $$ = $1 ; }
   | filter_declaration         { $$ = $1 ; }
   | end_filter_declaration     { $$ = $1 ; }
   | extends_declaration        { $$ = $1 ; }
   | macro_declaration          { $$ = $1 ; }
   | end_macro_declaration      { $$ = $1 ; }
   | import_declaration         { $$ = $1 ; }
  ;

block_declaration:
    T_BEGIN_BLOCK T_IDENTIFIER {
        auto node = make_shared<NamedBlockNode>($2) ;
        driver.addNode(node) ;
        driver.pushBlock(node);
    }
    | T_BEGIN_BLOCK T_IDENTIFIER expression {
        auto node = make_shared<NamedBlockNode>($2) ;
        driver.addNode(node) ;
        node->addChild(make_shared<SubTextNode>($3)) ;
    }
    ;

end_block_declaration:
    T_END_BLOCK { driver.popBlock() ; }
    | T_END_BLOCK T_IDENTIFIER { driver.popBlock() ; }
    ;

for_loop_declaration:
    T_FOR identifier_list T_IN expression {
        auto node = make_shared<ForLoopBlockNode>($2, $4) ;
        driver.addNode(node) ;
        driver.pushBlock(node);
    }

end_for_declaration:
    T_END_FOR { driver.popBlock() ; }

else_declaration:
    T_ELSE {
        if ( ForLoopBlockNode *p = dynamic_cast<ForLoopBlockNode *>(driver.stackTop().get()) )
            p->startElseBlock() ;
        else if ( IfBlockNode *p = dynamic_cast<IfBlockNode *>(driver.stackTop().get()) )
            p->addBlock(nullptr) ;
    }

if_declaration:
   T_IF expression {
        auto node = make_shared<IfBlockNode>($2) ;
        driver.addNode(node) ;
        driver.pushBlock(node);
   }

else_if_declaration:
    T_ELSE_IF expression {
        if ( IfBlockNode *p = dynamic_cast<IfBlockNode *>(driver.stackTop().get()) )
            p->addBlock($2) ;
    }

end_if_declaration:
    T_END_IF { driver.popBlock() ; }

set_declaration:
    T_SET T_IDENTIFIER T_ASSIGN expression  {
        auto node = make_shared<AssignmentBlockNode>($2, $4) ;
        driver.addNode(node) ;
        driver.pushBlock(node);
   }

end_set_declaration:
    T_END_SET { driver.popBlock() ; }

filter_declaration:
    T_FILTER filter  {

        auto node = make_shared<FilterBlockNode>($2) ;
            driver.addNode(node) ;
            driver.pushBlock(node);
    }

extends_declaration:
        T_EXTENDS expression  {
            auto node = make_shared<ExtensionBlockNode>($2) ;
            driver.addNode(node) ;
            driver.pushBlock(node);
       }

end_filter_declaration:
    T_END_FILTER { driver.popBlock() ; }

macro_declaration:
    T_MACRO T_IDENTIFIER T_LPAR identifier_list T_RPAR  {
        auto node = make_shared<MacroBlockNode>($2, $4) ;
        driver.addNode(node) ;
        driver.pushBlock(node);
        driver.addMacroBlock($2, node) ;
    }
    | T_MACRO T_IDENTIFIER T_LPAR T_RPAR  {
        auto node = make_shared<MacroBlockNode>($2) ;
        driver.addNode(node) ;
        driver.pushBlock(node);
        driver.addMacroBlock($2, node) ;
    }

end_macro_declaration:
    T_END_MACRO { driver.popBlock() ; }

import_declaration:
    T_IMPORT expression T_AS T_IDENTIFIER  {
            auto node = make_shared<ImportBlockNode>($2, $4) ;
            driver.addNode(node) ;
            driver.pushBlock(node);
        }
   | T_IMPORT T_SELF T_AS T_IDENTIFIER  {
            auto node = make_shared<ImportBlockNode>(nullptr, $4) ;
            driver.addNode(node) ;
            driver.pushBlock(node);
        }
identifier_list:
    T_IDENTIFIER                          { $$ = make_shared<IdentifierList>() ; $$->append($1) ; }
    | T_IDENTIFIER T_COMMA identifier_list  { $$ = $3 ; $3->prepend($1) ; }


expression:
          expression T_OR expression      { $$ = make_shared<BooleanOperator>( BooleanOperator::Or, $1, $3) ; }
        | expression T_AND expression     { $$ = make_shared<BooleanOperator>( BooleanOperator::And, $1, $3) ; }
        | expression T_EQUAL expression			{ $$ = make_shared<ComparisonPredicate>( ComparisonPredicate::Equal, $1, $3 ) ; }
        | expression T_NOT_EQUAL expression		{ $$ = make_shared<ComparisonPredicate>( ComparisonPredicate::NotEqual, $1, $3 ) ; }
        | expression T_LESS_THAN expression		{ $$ = make_shared<ComparisonPredicate>( ComparisonPredicate::Less, $1, $3 ) ; }
        | expression T_GREATER_THAN expression		{ $$ = make_shared<ComparisonPredicate>( ComparisonPredicate::Greater, $1, $3 ) ; }
        | expression T_LESS_THAN_OR_EQUAL expression	{ $$ = make_shared<ComparisonPredicate>( ComparisonPredicate::LessOrEqual, $1, $3 ) ; }
        | expression T_GREATER_THAN_OR_EQUAL expression	{ $$ = make_shared<ComparisonPredicate>( ComparisonPredicate::GreaterOrEqual, $1, $3 ) ; }
        | expression T_PLUS expression		{ $$ = make_shared<BinaryOperator>('+', $1, $3) ; }
        | expression T_MINUS expression		{ $$ = make_shared<BinaryOperator>('-', $1, $3) ; }
        | expression T_TILDE expression	{ $$ = make_shared<BinaryOperator>('~', $1, $3) ; }
        | expression T_STAR expression		{ $$ = make_shared<BinaryOperator>('*', $1, $3) ; }
        | expression T_DIV expression		{ $$ = make_shared<BinaryOperator>('/', $1, $3) ; }
        | T_PLUS expression  { $$ = make_shared<UnaryOperator>('+', $2) ; }
        | T_MINUS expression  { $$ = make_shared<UnaryOperator>('-', $2) ; }
        | T_LPAR expression T_RPAR { $$ = $2; }
        | expression T_QUESTION_MARK expression T_COLON expression { $$ = make_shared<TernaryExpressionNode>($1, $3, $5) ; }
        | expression  T_LEFT_BRACKET expression T_RIGHT_BRACKET     { $$ = make_shared<SubscriptIndexingNode>($1, $3) ; }
        | expression T_BAR filter                                 { $$ = make_shared<InvokeFilterNode>($1, $3) ; }
        | expression T_PERIOD T_IDENTIFIER                          { $$ = make_shared<AttributeIndexingNode>($1, $3) ; }
        | function_call  { $$ = $1 ; }
        | value { $$ = $1 ; }
        | T_IDENTIFIER { $$ = make_shared<IdentifierNode>($1) ; }

filter:
    T_IDENTIFIER                          { $$ = make_shared<FilterNode>($1, make_shared<FunctionArguments>()) ;    }
  | T_IDENTIFIER T_LPAR func_args T_RPAR  { $$ = make_shared<FilterNode>($1, $3) ; }

function_call:
    expression T_LPAR T_RPAR            { $$ = make_shared<InvokeFunctionNode>($1, make_shared<FunctionArguments>()) ; }
  | expression T_LPAR func_args T_RPAR  { $$ = make_shared<InvokeFunctionNode>($1, $3) ; }
	;

func_args:
    func_arg                    { $$ = make_shared<FunctionArguments>() ; $$->append($1) ; }
  | func_arg T_COMMA func_args  { $$ = $3 ;  $3->prepend($1) ; }


func_arg:
    expression                          { $$ = make_shared<FunctionArg>($1) ; }
    | T_IDENTIFIER T_ASSIGN expression  { $$ = make_shared<FunctionArg>($3, $1) ; }

value:
    T_STRING         { $$ = make_shared<LiteralNode>($1) ;  }
    | T_INTEGER      { $$ = make_shared<LiteralNode>($1) ; }
    | T_FLOAT        { $$ = make_shared<LiteralNode>($1) ; }
    | object         { $$ = $1 ; }
    | array          { $$ = $1 ; }
    | T_TRUE         { $$ = make_shared<LiteralNode>(true) ; }
    | T_FALSE        { $$ = make_shared<LiteralNode>(false) ; }
    | T_NULL         { $$ = make_shared<LiteralNode>(Variant::null()) ; }

array:
    T_LEFT_BRACKET expression_list T_RIGHT_BRACKET { $$ = make_shared<ArrayNode>($2) ; }

object:
    T_LEFT_BRACE key_val_list T_RIGHT_BRACE { $$ = make_shared<DictionaryNode>($2) ; }

expression_list:
    expression {  $$ = make_shared<ExpressionList>() ;
                   $$->append($1) ;
                }
    | expression T_COMMA expression_list  {  $$ = $3 ;  $3->prepend($1) ; }

key_val_list:
    key_val                             {   $$ = make_shared<KeyValList>() ;
                                            $$->append($1) ;
                                        }
    | key_val T_COMMA key_val_list        {  $$ = $3 ; $3->prepend($1) ;
                                        }

key_val:
    T_STRING T_COLON expression { $$ = make_shared<KeyValNode>($1, $3) ; }
    | T_IDENTIFIER T_COLON expression { $$ = make_shared<KeyValNode>($1, $3) ; }


%%
//#define YYDEBUG 1

#include "scanner.hpp"

// We have to implement the error function
void yy::Parser::error(const yy::Parser::location_type &loc, const string &msg) {
	driver.error(loc, msg) ;
}

// Now that we have the Parser declared, we can declare the Scanner and implement
// the yylex function

static yy::Parser::symbol_type yylex(TemplateParser &driver, yy::Parser::location_type &loc) {
    return driver.scanner().lex(&loc);
}


