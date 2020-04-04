%skeleton "lalr1.cc"
%require "3.4"
%defines
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
	#include <string>
	#include <memory>
	#include "ast.hpp"
	class driver;
}

%param { driver& drv }
%locations
%define parse.error verbose

%code {
	#include "driver.hh"
	yy::parser::symbol_type yylex(driver& drv);
	std::shared_ptr<NodeList> programList;
}

%define api.token.prefix {TOK_}
%token
  END  0	"end of file"
  BIT_OR	"|"
  BIT_XOR	"^"
  BIT_AND	"&"
  LESS		"<"
  GRE		">"
  LPAREN	"("
  RPAREN	")"
  LCB		"{"
  RCB		"}"
  LSB		"["
  RSB		"]"
  DOT		"."
  MINUS		"-"
  LOG_NOT	"!"
  BIT_NOT	"~"
  ADD		"+"
  MUL		"*"
  DIV		"/"
  MOD		"%"
  COMMA		","
  ASS		"="
  SEMICOLON ";"
  COLON		":"
  UNDERSCORE "_"
;
%type  <std::shared_ptr<AST>> top return_statement assignment_expression statement case_statement elseOpt
%type  <std::shared_ptr<AST>> nondeterministic_and_expression nondeterministic_or_expression
%type  <std::shared_ptr<AST>> nondeterministic_limit_expression nondeterministic_not_expression logical_OR_expression
%type  <std::shared_ptr<AST>> logical_AND_expression inclusive_OR_expression exclusive_OR_expression binary_and_expression
%type  <std::shared_ptr<AST>> equality_expression relational_expression shift_expression additive_expression 
%type  <std::shared_ptr<AST>> multiplicative_expression unary_expression postfix_expression primary_expression lambda
%type  <std::shared_ptr<AST>> topOrEmpty emptyOrArgList
%type  <std::shared_ptr<NodeList>> topList structList dataList case_list expressionList typeList argList
%type  <std::shared_ptr<ID>> id
%type  <std::shared_ptr<Argument>> argument
%type  <std::shared_ptr<Type>> type
%type  <std::string> oper
%token <std::string> IDENTIFIER STRING_LITERAL
%token <double> F64_LITERAL
%token <long long> I64_LITERAL
%token <char> CHAR_LITERAL
%token <bool> TRUE FALSE
%token I64 F64 STRING CHAR BOOL VOID
%token FN STRUCT DATA OPERATOR RET YIELD SWITCH CASE EVERY WHILE DO IF ELSE CONTINUE BREAK ARROW
%token LIMIT NDTOR NDTAND NDTNOT LOGOR LOGAND EQ NE LE GE SHL SHR NIL SUCCESS FAIL NOP LAMBDA_BEGINER

%%

%start program;

program: topList {programList = $1;}
;

topList: top {
		$$ = std::make_shared<NodeList>(drv.location, NodeListType::SEPARATOR, $1);
	}
| topList ";" top {$1->addNode($3); $$ = $1;}
;

top: return_statement {$$ = $1;}
| STRUCT IDENTIFIER "{" structList "}" {
	$$ = std::make_shared<Struct>(drv.location, StructType::STR, $2, $4);
}
| DATA IDENTIFIER "{" dataList "}" {
	$$ = std::make_shared<Struct>(drv.location, StructType::DATA, $2, $4);
}
| FN IDENTIFIER "(" emptyOrArgList ")" ":" type "{" topList "}" {
	$$ = std::make_shared<Function>(drv.location, $2, $4, $7, $9);
}
| OPERATOR oper "(" emptyOrArgList ")" ":" type "{" topList "}" {
	$$ = std::make_shared<Function>(drv.location, $2, $4, $7, $9);
}
;

structList: argument ";" {
		$$ = std::make_shared<NodeList>(drv.location, NodeListType::STRUCTLIST, $1);
	}
| structList argument ";" {$1->addNode($2); $$ = $1;}
;

dataList: id ";" {
		$$ = std::make_shared<NodeList>(drv.location, NodeListType::DATALIST, $1);
	}
| dataList id ";" {$1->addNode($2); $$ = $1;}
;

oper: '+' {$$ = "?ADD";}
| '-' {$$ = "?MINUS";}
| '.' {$$ = "?DOT";}
| '(' ')' {$$ = "?CALL";}
| '{' '}' {$$ = "?OBJECT";}
| '[' ']' {$$ = "?ARRAY_ELEMENT";}
| '!' {$$ = "?NEG";}
| '~' {$$ = "?BITNEG";}
| '*' {$$ = "?MUL";}
| '/' {$$ = "?DIV";}
| '%' {$$ = "?MOD";}
| '>' {$$ = "?GRE";}
| '<' {$$ = "?LES";}
| '&' {$$ = "?BIT_AND";}
| '^' {$$ = "?BIT_XOR";}
| '|' {$$ = "?BIT_OR";}
| '=' {$$ = "?ASS";}
| GE {$$ = "?GEQ";}
| LE {$$ = "?LEQ";}
| SHL {$$ = "?SHL";}
| SHR {$$ = "?SHR";}
;

return_statement: assignment_expression {$$ = $1;}
| RET assignment_expression {
		$$ = std::make_shared<Return>(drv.location, ReturnType::RET, $2);
	}
| YIELD assignment_expression {
		$$ = std::make_shared<Return>(drv.location, ReturnType::YIELD, $2);
	}
;

assignment_expression: statement {$$ = $1;} //TODO only id & primary expr on left side
| assignment_expression "=" statement {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::ASS, $1, $3);
	}
;

statement: expressionList {
        if($1->hasOneElement()) $$ = $1->giveMeOnlyElem();
        else $$ = $1;
    }
| IF "(" top ")" "{" topList "}" elseOpt {
		$$ = std::make_shared<Statement>(drv.location, StatementType::IF, $3, $6, $8);
	}
| EVERY "(" top ")" "{" topList "}" {
		$$ = std::make_shared<Statement>(drv.location, StatementType::EVERY, $3, $6);
	}
| WHILE "(" top ")" "{" topList "}" {
		$$ = std::make_shared<Statement>(drv.location, StatementType::WHILE, $3, $6);
	}
| DO "{" topList "}" WHILE "(" top ")" {
		$$ = std::make_shared<Statement>(drv.location, StatementType::DO_WHILE, $7, $3);
	}
| SWITCH "(" top ")" "{" case_list "}" {
		$$ = std::make_shared<Statement>(drv.location, StatementType::SWITCH, $3, $6);
	}
| CONTINUE {$$ = std::make_shared<Special>(drv.location, SpecialType::CONTINUE);}
| BREAK {$$ = std::make_shared<Special>(drv.location, SpecialType::BREAK);}
;

case_list: case_statement {
		$$ = std::make_shared<NodeList>(drv.location, NodeListType::CASE_LIST, $1);
	}
| case_list case_statement {$1->addNode($2); $$ = $1;}
;

case_statement: CASE top ARROW "{" topList "}" {
		$$ = std::make_shared<Statement>(drv.location, StatementType::CASE, $2, $5);
	}
;

elseOpt: %empty {$$ = std::make_shared<Special>(drv.location, SpecialType::EMPTY);}
| ELSE "{" topList "}" {$$ = $3;}
;

expressionList: nondeterministic_and_expression {
		$$ = std::make_shared<NodeList>(drv.location, NodeListType::COMMA, $1);
	}
| expressionList "," nondeterministic_and_expression {$1->addNode($3); $$ = $1;}
;

nondeterministic_and_expression: nondeterministic_or_expression {$$ = $1;}
| nondeterministic_and_expression NDTAND nondeterministic_or_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::NDT_AND, $1, $3);
	}
;
		
nondeterministic_or_expression: nondeterministic_limit_expression {$$ = $1;}
| nondeterministic_or_expression NDTOR nondeterministic_limit_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::NDT_OR, $1, $3);
	}
;

nondeterministic_limit_expression: nondeterministic_not_expression {$$ = $1;}
| nondeterministic_limit_expression LIMIT nondeterministic_not_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::LIMIT, $1, $3);
	}
;

nondeterministic_not_expression: logical_OR_expression {$$ = $1;}
| NDTNOT logical_OR_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::NDT_NOT, $2);
	}
;

logical_OR_expression: logical_AND_expression {$$ = $1;}
| logical_OR_expression LOGOR logical_AND_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::LOG_OR, $1, $3);
	}
;

logical_AND_expression: inclusive_OR_expression {$$ = $1;}
| logical_AND_expression LOGAND inclusive_OR_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::LOG_AND, $1, $3);
	}
;

inclusive_OR_expression: exclusive_OR_expression {$$ = $1;}
| inclusive_OR_expression "|" exclusive_OR_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::BIT_OR, $1, $3);
	}
;

exclusive_OR_expression: binary_and_expression {$$ = $1;}
| exclusive_OR_expression "^" binary_and_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::BIT_XOR, $1, $3);
	}
;

binary_and_expression: equality_expression {$$ = $1;}
| binary_and_expression "&" equality_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::BIT_AND, $1, $3);
	}
;

equality_expression: relational_expression {$$ = $1;}
| equality_expression EQ relational_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::EQU, $1, $3);
	}
| equality_expression NE relational_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::NEQU, $1, $3);
	}
;

relational_expression: shift_expression {$$ = $1;}
|   relational_expression "<" shift_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::LESS, $1, $3);
	}
|   relational_expression ">" shift_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::GREATER, $1, $3);
	}
|   relational_expression LE shift_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::LEQ, $1, $3);
	}
|   relational_expression GE shift_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::GEQ, $1, $3);
	}
;

shift_expression: additive_expression {$$ = $1;}
| shift_expression SHL additive_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::SHL, $1, $3);
	}
| shift_expression SHR additive_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::SHR, $1, $3);
	}
;

additive_expression: multiplicative_expression {$$ = $1;}
| additive_expression "+" multiplicative_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::ADD, $1, $3);
	}
| additive_expression "-" multiplicative_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::SUB, $1, $3);
	}
;

multiplicative_expression: unary_expression {$$ = $1;}
| multiplicative_expression "*" unary_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::MUL, $1, $3);
	}
| multiplicative_expression "/" unary_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::DIV, $1, $3);
	}
| multiplicative_expression "%" unary_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::MOD, $1, $3);
	}
;

unary_expression: postfix_expression {$$ = $1;}
| "-" unary_expression {$$ = std::make_shared<Expression>(drv.location, ExpressionType::MINUS, $2);}
| "!" unary_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::LOG_NOT, $2);
	}
| "~" unary_expression {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::BIT_NOT, $2);
	}
;

postfix_expression: primary_expression {$$ = $1;}
| postfix_expression "." id {
	$$ = std::make_shared<Expression>(drv.location, ExpressionType::DOT, $1, $3);
}
| postfix_expression "[" topOrEmpty "]" {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::ARRAY_ELEM, $1, $3);
	}
| postfix_expression "(" topOrEmpty ")" {
		$$ = std::make_shared<Expression>(drv.location, ExpressionType::CALL, $1, $3);
	}
 // | postfix_expression "{" topOrEmpty '}'
;

primary_expression: id {$$ = $1;}
| I64_LITERAL {$$ = std::make_shared<Literal>(drv.location, $1);}
| F64_LITERAL {$$ = std::make_shared<Literal>(drv.location, $1);}
| CHAR_LITERAL {$$ = std::make_shared<Literal>(drv.location, $1);}
| FALSE {$$ = std::make_shared<Literal>(drv.location, false);}
| TRUE {$$ = std::make_shared<Literal>(drv.location, true);}
| STRING_LITERAL {$$ = std::make_shared<Literal>(drv.location, $1);}
| "(" top ")" {$$=$2;}
| "{" top "}" {$$ = std::make_shared<Object>(drv.location, $2);}
| "[" top "]" {$$ = std::make_shared<List>(drv.location, $2);}
| NIL {$$ = std::make_shared<Special>(drv.location, SpecialType::NIL);}
| SUCCESS {$$ = std::make_shared<Special>(drv.location, SpecialType::SUCCESS);}
| FAIL {$$ = std::make_shared<Special>(drv.location, SpecialType::FAIL);}
| NOP {$$ = std::make_shared<Special>(drv.location, SpecialType::NOP);}
| "_" {$$ = std::make_shared<Special>(drv.location, SpecialType::UNDERSCORE);}
| lambda {$$=$1;}
;

id: IDENTIFIER {$$ = std::make_shared<ID>(drv.location, $1);}
;

topOrEmpty: top {$$=$1;}
| %empty {$$ = std::make_shared<Special>(drv.location, SpecialType::EMPTY);}
;

lambda: LAMBDA_BEGINER "(" emptyOrArgList ")" ":" type "{" topList "}" {
	$$ = std::make_shared<Function>(drv.location, "?lambda", $3, $6, $8);
}
;

type: IDENTIFIER {$$ = std::make_shared<Type>(drv.location, TypeType::IDENT, $1);}
| I64 {$$ = std::make_shared<Type>(drv.location, TypeType::I64);}
| F64 {$$ = std::make_shared<Type>(drv.location, TypeType::F64);}
| STRING {$$ = std::make_shared<Type>(drv.location, TypeType::STRING);}
| CHAR {$$ = std::make_shared<Type>(drv.location, TypeType::CHAR);}
| BOOL {$$ = std::make_shared<Type>(drv.location, TypeType::BOOL);}
| VOID {$$ = std::make_shared<Type>(drv.location, TypeType::VOID);}
| "[" type "]" {$$ = std::make_shared<Type>(drv.location, TypeType::ARRAY, $2);}
| "{" typeList "}" {$$ = std::make_shared<Type>(drv.location, TypeType::OBJECT, $2);}
| "(" typeList ":" type ")" {$$ = std::make_shared<Type>(drv.location, TypeType::FUNCTION, $2, $4);}
;

typeList: type {
		$$ = std::make_shared<NodeList>(drv.location, NodeListType::TYPELIST, $1);
	}
| typeList "," type {$1->addNode($3); $$ = $1;}
;

emptyOrArgList: %empty {$$ = std::make_shared<Special>(drv.location, SpecialType::EMPTY);}
| argList {$$=$1;}
;

argList: argument {
		$$ = std::make_shared<NodeList>(drv.location, NodeListType::ARGLIST, $1);
	}
| argList "," argument {$1->addNode($3); $$ = $1;}
;

argument: IDENTIFIER ":" type {$$ = std::make_shared<Argument>(drv.location, $1, $3);}
;

%%

void yy::parser::error (const location_type& l, const std::string& m){
	std::cerr << l << ": " << m << '\n';
}

extern void semanticAnalyzerMain(std::shared_ptr<NodeList> programList);
int main (int argc, char *argv[]){
	driver drv;
	drv.parse(argv[1]);
    std::cout<<"\tPARSE DONE!\n";
    std::cout<<(programList->print())<<'\n';
	semanticAnalyzerMain(programList);
	return 0;
}