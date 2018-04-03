%require "3.0.2"
%skeleton "lalr1.cc"

%defines
%locations


%define parser_class_name {Parser}
%define api.token.constructor
%define api.value.type variant
%define api.token.prefix {TOK_}

%param { TwigParser &driver }
%param { Parser::location_type &loc }

%define parse.trace
%define parse.error verbose

%code requires {
#include <ast.hpp>
class TwigParser ;


namespace wspp { namespace twig { namespace detail {
    using expr_list_t = std::deque<ExpressionNodePtr> ;
    using key_val_t = std::pair<std::string, ExpressionNodePtr> ;
    using key_val_list_t = std::deque<key_val_t> ;
    using identifier_list_t = std::deque<std::string> ;
    using key_alias_t = std::pair<std::string, std::string> ;
    using key_alias_list_t = std::deque<key_alias_t> ;
}}}

}

%code {
#include <parser.hpp>
#include <ast.hpp>

using namespace wspp::twig::detail ;
using namespace std ;

static yy::Parser::symbol_type yylex(TwigParser &driver, yy::Parser::location_type &loc);

}

/* literal keyword tokens */
%token T_NOT            "not"
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
%token T_MOD            "%"

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
%token T_EMBED          "embed"
%token T_END_EMBED      "endembed"
%token T_MACRO          "macro"
%token T_END_MACRO      "end macro"
%token T_WITH           "with"
%token T_END_WITH       "end with"
%token T_ONLY           "only"
%token T_INCLUDE        "include"
%token T_IGNORE         "ignore"
%token T_MISSING        "missing"
%token T_FROM           "from"
%token T_SELF           "_self"
%token T_AS             "as"
%token T_IMPORT         "import"
%token T_MATCHES        "matches"
%token T_ASSIGN         "="
%token T_IN             "in"
%token T_IS             "is"
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

%token <std::string> T_IDENTIFIER       "identifier";
%token <std::string> T_RAW_CHARACTERS  "raw characters";
%token <int64_t>     T_INTEGER         "integer";
%token <double>      T_FLOAT           "float";
%token <std::string> T_STRING          "string literal";

%type <wspp::twig::detail::ExpressionNodePtr> expression value array object function_call with_expression filter_invoke test_call
%type <wspp::twig::detail::expr_list_t> expression_list
%type <wspp::twig::detail::key_val_list_t> key_val_list func_args
%type <wspp::twig::detail::key_val_t> key_val func_arg
%type <wspp::twig::detail::identifier_list_t> identifier_list
%type <wspp::twig::detail::ContentNodePtr> block_tag sub_tag tag_or_chars tag_declaration
%type <wspp::twig::detail::ContentNodePtr> block_declaration end_block_declaration for_loop_declaration end_for_declaration else_declaration if_declaration
%type <wspp::twig::detail::ContentNodePtr> else_if_declaration end_if_declaration set_declaration end_set_declaration filter_declaration end_filter_declaration
%type <wspp::twig::detail::ContentNodePtr> extends_declaration macro_declaration end_macro_declaration import_declaration
%type <wspp::twig::detail::ContentNodePtr> include_declaration with_declaration end_with_declaration
%type <bool> ignore_missing_flag only_flag
%type <wspp::twig::detail::key_alias_list_t> import_list
%type <wspp::twig::detail::key_alias_t> import_key_alias

/*operators */

%right T_QUESTION_MARK T_COLON
%left T_NOT
%left T_OR
%left T_AND
%nonassoc T_LESS_THAN T_GREATER_THAN T_LESS_THAN_OR_EQUAL T_GREATER_THAN_OR_EQUAL T_EQUAL T_NOT_EQUAL
%nonassoc T_IN
%nonassoc T_MATCHES
%left T_PLUS T_MINUS T_TILDE
%left T_STAR T_DIV
%left T_MOD
%left T_UMINUS T_NEG
%left T_IS
%left T_BAR
%nonassoc T_NO_ARGS
%right T_LPAR
%right T_LEFT_BRACKET T_PERIOD
%left T_ASSIGN T_COMMA

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
    T_START_BLOCK_TAG tag_declaration T_END_BLOCK_TAG
    | T_START_BLOCK_TAG_TRIM tag_declaration T_END_BLOCK_TAG
    | T_START_BLOCK_TAG_TRIM tag_declaration T_END_BLOCK_TAG_TRIM { driver.trimWhiteAfter() ;  }
    | T_START_BLOCK_TAG tag_declaration T_END_BLOCK_TAG_TRIM { driver.trimWhiteAfter() ;}
;

sub_tag:
    T_DOUBLE_LEFT_BRACE expression T_DOUBLE_RIGHT_BRACE {
        driver.addNode(make_shared<SubTextNode>($2)) ;
    }
    | T_DOUBLE_LEFT_BRACE_TRIM expression T_DOUBLE_RIGHT_BRACE {
        driver.trimWhiteBefore() ;
        driver.addNode(make_shared<SubTextNode>($2)) ;
    }
    | T_DOUBLE_LEFT_BRACE expression T_DOUBLE_RIGHT_BRACE_TRIM {
        driver.addNode(make_shared<SubTextNode>($2)) ;
        driver.trimWhiteAfter() ;
    }
    | T_DOUBLE_LEFT_BRACE_TRIM expression T_DOUBLE_RIGHT_BRACE_TRIM {
        driver.trimWhiteBefore() ;
        driver.addNode(make_shared<SubTextNode>($2)) ;
        driver.trimWhiteAfter() ;
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
   | include_declaration        { $$ = $1 ; }
   | with_declaration           { $$ = $1 ; }
   | end_with_declaration       { $$ = $1 ; }
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
    T_END_BLOCK { driver.popBlock("block") ; }
    | T_END_BLOCK T_IDENTIFIER { driver.popBlock("block") ; }
    ;

for_loop_declaration:
    T_FOR identifier_list T_IN expression {
        auto node = make_shared<ForLoopBlockNode>(std::move($2), $4) ;
        driver.addNode(node) ;
        driver.pushBlock(node);
    }
  | T_FOR identifier_list T_IN expression T_IF expression {
    auto node = make_shared<ForLoopBlockNode>(std::move($2), $4, $6) ;
    driver.addNode(node) ;
    driver.pushBlock(node);
}


end_for_declaration:
    T_END_FOR { driver.popBlock("for") ; }

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
    T_END_IF { driver.popBlock("if") ; }

set_declaration:
    T_SET T_IDENTIFIER T_ASSIGN expression  {
        auto node = make_shared<AssignmentBlockNode>($2, $4) ;
        driver.addNode(node) ;
        driver.pushBlock(node);
   }

end_set_declaration:
    T_END_SET { driver.popBlock("set") ; }

filter_declaration:
    T_FILTER T_IDENTIFIER  {

        auto node = make_shared<FilterBlockNode>($2) ;
            driver.addNode(node) ;
            driver.pushBlock(node);
    }
    | T_FILTER T_IDENTIFIER T_LPAR func_args T_RPAR {

        auto node = make_shared<FilterBlockNode>($2, std::move($4)) ;
            driver.addNode(node) ;
            driver.pushBlock(node);
    }


end_filter_declaration:
    T_END_FILTER { driver.popBlock("filter") ; }

extends_declaration:
        T_EXTENDS expression  {
            auto node = make_shared<ExtensionBlockNode>($2) ;
            driver.addNode(node) ;
            driver.pushBlock(node);
       }

macro_declaration:
    T_MACRO T_IDENTIFIER T_LPAR identifier_list T_RPAR  {
        auto node = make_shared<MacroBlockNode>($2, std::move($4)) ;
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
    T_END_MACRO { driver.popBlock("macro") ; }

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
   | T_FROM expression T_IMPORT import_list  {
                auto node = make_shared<ImportBlockNode>($2, std::move($4)) ;
                driver.addNode(node) ;
                driver.pushBlock(node);
            }

import_list:
    import_key_alias                        { $$.push_back(std::move($1)); }
    | import_list T_COMMA import_key_alias  { $1.push_back(std::move($3)) ; std::swap($$, $1) ; }

import_key_alias:
    T_IDENTIFIER                        { $$ = make_pair($1, std::string()) ; }
    | T_IDENTIFIER T_AS T_IDENTIFIER    { $$ = make_pair($1, $3) ; }

include_declaration:
    T_INCLUDE expression ignore_missing_flag with_expression only_flag {
        auto node = make_shared<IncludeBlockNode>($2, $3, $4, $5) ;
        driver.addNode(node) ;
    }

ignore_missing_flag:
    %empty { $$ = false ; }
    | T_IGNORE T_MISSING { $$ = true ; }

with_expression:
    %empty { $$ = nullptr ; }
    | T_WITH expression { $$ = $2 ; }

only_flag:
    %empty { $$ = false ; }
    | T_ONLY { $$ = true ; }

with_declaration:
    T_WITH only_flag {
        auto node = make_shared<WithBlockNode>(nullptr, $2) ;
        driver.addNode(node) ;
        driver.pushBlock(node) ;
        $$ = node ;
    }
    | T_WITH expression only_flag {
        auto node = make_shared<WithBlockNode>($2, $3) ;
        driver.addNode(node) ;
        driver.pushBlock(node) ;
        $$ = node ;
    }

end_with_declaration:
    T_END_WITH { driver.popBlock("with") ; }

identifier_list:
    T_IDENTIFIER                            {  $$.push_back($1) ; }
    | identifier_list T_COMMA T_IDENTIFIER  { $1.push_back($3) ; std::swap($$, $1) ; }


expression:
          T_NOT expression %prec T_NEG                     { $$ = make_shared<BooleanOperator>( BooleanOperator::Not, $2, nullptr) ; }
        | expression T_OR expression            { $$ = make_shared<BooleanOperator>( BooleanOperator::Or, $1, $3) ; }
        | expression T_AND expression           { $$ = make_shared<BooleanOperator>( BooleanOperator::And, $1, $3) ; }
        | expression T_EQUAL expression			{ $$ = make_shared<ComparisonPredicate>( ComparisonPredicate::Equal, $1, $3 ) ; }
        | expression T_NOT_EQUAL expression		{ $$ = make_shared<ComparisonPredicate>( ComparisonPredicate::NotEqual, $1, $3 ) ; }
        | expression T_LESS_THAN expression		{ $$ = make_shared<ComparisonPredicate>( ComparisonPredicate::Less, $1, $3 ) ; }
        | expression T_GREATER_THAN expression		{ $$ = make_shared<ComparisonPredicate>( ComparisonPredicate::Greater, $1, $3 ) ; }
        | expression T_LESS_THAN_OR_EQUAL expression	{ $$ = make_shared<ComparisonPredicate>( ComparisonPredicate::LessOrEqual, $1, $3 ) ; }
        | expression T_GREATER_THAN_OR_EQUAL expression	{ $$ = make_shared<ComparisonPredicate>( ComparisonPredicate::GreaterOrEqual, $1, $3 ) ; }
        | expression T_PLUS expression          { $$ = make_shared<BinaryOperator>('+', $1, $3) ; }
        | expression T_MINUS expression         { $$ = make_shared<BinaryOperator>('-', $1, $3) ; }
        | expression T_TILDE expression         { $$ = make_shared<BinaryOperator>('~', $1, $3) ; }
        | expression T_STAR expression          { $$ = make_shared<BinaryOperator>('*', $1, $3) ; }
        | expression T_DIV expression           { $$ = make_shared<BinaryOperator>('/', $1, $3) ; }
        | expression T_MOD expression           { $$ = make_shared<BinaryOperator>('%', $1, $3) ; }
        | T_PLUS expression %prec T_UMINUS                    { $$ = make_shared<UnaryOperator>('+', $2) ; }
        | T_MINUS expression %prec T_UMINUS                   { $$ = make_shared<UnaryOperator>('-', $2) ; }
        | T_LPAR expression T_RPAR { std::swap($$, $2); }
        | expression T_QUESTION_MARK expression T_COLON expression  { $$ = make_shared<TernaryExpressionNode>($1, $3, $5) ; }
        | expression T_QUESTION_MARK expression { $$ = make_shared<TernaryExpressionNode>($1, $3, nullptr) ; }
        | expression  T_LEFT_BRACKET expression T_RIGHT_BRACKET     { $$ = make_shared<SubscriptIndexingNode>($1, $3) ; }
        | filter_invoke                                             { $$ = $1 ; }
        | test_call                                                 { $$ = $1 ; }
        | expression T_PERIOD T_IDENTIFIER                          { $$ = make_shared<AttributeIndexingNode>($1, $3) ; }
        | function_call                                             { $$ = $1 ; }
        | value                                                     { $$ = $1 ; }
        | expression T_IN expression                                { $$ = make_shared<ContainmentNode>($1, $3, true) ; }
        | expression T_NOT T_IN expression                          { $$ = make_shared<ContainmentNode>($1, $4, false) ; }
        | expression T_MATCHES T_STRING                             { $$ = make_shared<MatchesNode>($1, $3, true) ; }
        | expression T_NOT T_MATCHES T_STRING                       { $$ = make_shared<MatchesNode>($1, $4, false) ; }

filter_invoke:
    expression T_BAR T_IDENTIFIER %prec T_NO_ARGS            { $$ = make_shared<InvokeFilterNode>($1, $3) ; }
  | expression T_BAR T_IDENTIFIER T_LPAR func_args T_RPAR    { $$ = make_shared<InvokeFilterNode>($1, $3, std::move($5)) ; }

function_call:
    expression T_LPAR T_RPAR            { $$ = make_shared<InvokeFunctionNode>($1) ; }
  | expression T_LPAR func_args T_RPAR  { $$ = make_shared<InvokeFunctionNode>($1, std::move($3)) ; }
	;
test_call:
   expression T_IS T_IDENTIFIER   %prec T_NO_ARGS                  { $$ = make_shared<InvokeTestNode>($1, $3, key_val_list_t{}, true) ; }
   | expression T_IS T_NOT T_IDENTIFIER %prec T_NO_ARGS            { $$ = make_shared<InvokeTestNode>($1, $4, key_val_list_t{}, false) ; }
   | expression T_IS T_IDENTIFIER T_LPAR func_args T_RPAR          { $$ = make_shared<InvokeTestNode>($1, $3, std::move($5), true) ; }
   | expression T_IS T_NOT T_IDENTIFIER T_LPAR func_args T_RPAR    { $$ = make_shared<InvokeTestNode>($1, $4, std::move($6), false) ; }

func_args:
    func_arg                    { $$.push_back(std::move($1)) ; }
  | func_args T_COMMA func_arg  { $1.push_back(std::move($3)) ;  std::swap($$, $1) ; }


func_arg:
    expression                          { $$ = make_pair(std::string(), $1) ; }
    | T_IDENTIFIER T_ASSIGN expression  { $$ = make_pair($1, $3) ; }

value:
    T_IDENTIFIER     { $$ = make_shared<IdentifierNode>($1) ; }
    | T_STRING         { $$ = make_shared<LiteralNode>($1) ;  }
    | T_INTEGER      { $$ = make_shared<LiteralNode>($1) ; }
    | T_FLOAT        { $$ = make_shared<LiteralNode>($1) ; }
    | object         { std::swap($$, $1) ; }
    | array          { std::swap($$, $1) ; }
    | T_TRUE         { $$ = make_shared<LiteralNode>(true) ; }
    | T_FALSE        { $$ = make_shared<LiteralNode>(false) ; }
    | T_NULL         { $$ = make_shared<LiteralNode>(Variant::null()) ; }

array:
    T_LEFT_BRACKET expression_list T_RIGHT_BRACKET { $$ = make_shared<ArrayNode>(std::move($2)) ; }

object:
    T_LEFT_BRACE key_val_list T_RIGHT_BRACE { $$ = make_shared<DictionaryNode>(std::move($2)) ; }

expression_list:
    expression {  $$.push_back($1) ;  }
    | expression_list T_COMMA expression  { $1.push_back($3) ; std::swap($$, $1) ; }

key_val_list:
    key_val      {   $$.emplace_back($1) ;  }
    | key_val_list T_COMMA key_val  {  $1.push_back(std::move($3)) ; std::swap($$, $1) ; }

key_val:
    T_STRING T_COLON expression { $$ = std::make_pair($1, $3) ; }
    | T_IDENTIFIER T_COLON expression { $$ = std::make_pair($1, $3) ; }

%%
#define YYDEBUG 1

#include "scanner.hpp"

// We have to implement the error function
void yy::Parser::error(const yy::Parser::location_type &loc, const string &msg) {
	driver.error(loc, msg) ;
}

// Now that we have the Parser declared, we can declare the Scanner and implement
// the yylex function

static yy::Parser::symbol_type yylex(TwigParser &driver, yy::Parser::location_type &loc) {
    return driver.scanner().lex(&loc);
}


