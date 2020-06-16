%skeleton "lalr1.cc"
%require "3.4"
%defines
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
	#include <string>
	#include <memory>
	#include <iostream>
	#include <bits/stdc++.h>
	#include <stdlib.h>
	#include "ast/ast.hpp"
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
;
%type  <std::shared_ptr<AST>> top programTop assignment_expression statement elseOpt
%type  <std::shared_ptr<AST>> nondeterministic_and_expression
%type  <std::shared_ptr<AST>> nondeterministic_not_expression logical_OR_expression
%type  <std::shared_ptr<AST>> logical_AND_expression inclusive_OR_expression exclusive_OR_expression binary_and_expression
%type  <std::shared_ptr<AST>> equality_expression relational_expression shift_expression additive_expression 
%type  <std::shared_ptr<AST>> multiplicative_expression unary_expression postfix_expression primary_expression
%type  <std::shared_ptr<AST>> topOrEmpty emptyOrArgList
%type  <std::shared_ptr<NodeList>> programL topList structList expressionList argList
%type  <std::shared_ptr<ID>> id
%token <std::string> IDENTIFIER STRING_LITERAL
%token <double> F64_LITERAL
%token <long long> I64_LITERAL
%token <char> CHAR_LITERAL
%token <bool> TRUE FALSE
%token FN STRUCT RET YIELD EVERY WHILE DO IF ELSE CONTINUE BREAK
%token NDTOR NDTAND NDTNOT LOGOR LOGAND EQ NE LE GE SHL SHR NIL FAIL

%%

%start program;

program: programL {programList = $1;}
;

programL: programL programTop {$1->addNode($2); $$ = $1;}
| programTop {
		$$ = std::make_shared<NodeList>(drv.location, NodeType::PROGRAMLIST, $1);
	}
;

programTop: STRUCT IDENTIFIER "{" structList "}" {
	$$ = std::make_shared<Struct>(drv.location, NodeType::STR, $2, $4);
}
| FN IDENTIFIER "(" emptyOrArgList ")" "{" topList "}" {
	$$ = std::make_shared<Function>(drv.location, NodeType::FUNCTION, $2, $4, $7);
}
;

structList: id  {
		$$ = std::make_shared<NodeList>(drv.location, NodeType::STRUCTLIST, $1);
	}
| structList "," id {$1->addNode($3); $$ = $1;}
;

topList: top {
		$$ = std::make_shared<NodeList>(drv.location, NodeType::SEPARATOR, $1);
	}
| topList ";" top {$1->addNode($3); $$ = $1;}
;

top: assignment_expression {$$ = $1;}
| RET assignment_expression {
		$$ = std::make_shared<Return>(drv.location, NodeType::RET, $2);
	}
| YIELD assignment_expression {
		$$ = std::make_shared<Return>(drv.location, NodeType::YIELD, $2);
	}
;

assignment_expression: statement {$$ = $1;}
| assignment_expression "=" statement {
		$$ = std::make_shared<Expression>(drv.location, NodeType::ASS, $1, $3);
	}
;

statement: expressionList {
        if($1->nrOfElements() == 1) $$ = $1->giveMeOnlyElem();
        else $$ = $1;
    }
| IF "(" top ")" "{" topList "}" elseOpt {
		$$ = std::make_shared<Statement>(drv.location, NodeType::IF, $3, $6, $8);
	}
| EVERY "(" top ")" "{" topList "}" {
		$$ = std::make_shared<Statement>(drv.location, NodeType::EVERY, $3, $6);
	}
| WHILE "(" top ")" "{" topList "}" {
		$$ = std::make_shared<Statement>(drv.location, NodeType::WHILE, $3, $6);
	}
| DO "{" topList "}" WHILE "(" top ")" {
		$$ = std::make_shared<Statement>(drv.location, NodeType::DO_WHILE, $7, $3);
	}
| CONTINUE {$$ = std::make_shared<Special>(drv.location, NodeType::CONTINUE);}
| BREAK {$$ = std::make_shared<Special>(drv.location, NodeType::BREAK);}
;

elseOpt: %empty {$$ = std::make_shared<Special>(drv.location, NodeType::EMPTY);}
| ELSE "{" topList "}" {$$ = $3;}
;

expressionList: nondeterministic_and_expression {
		$$ = std::make_shared<NodeList>(drv.location, NodeType::COMMA, $1);
	}
| expressionList "," nondeterministic_and_expression {$1->addNode($3); $$ = $1;}
;

nondeterministic_and_expression: nondeterministic_not_expression  {$$ = $1;}
| nondeterministic_and_expression NDTAND nondeterministic_not_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::NDT_AND, $1, $3);
	}
;

nondeterministic_not_expression: logical_OR_expression {$$ = $1;}
| NDTNOT logical_OR_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::NDT_NOT, $2);
	}
;

logical_OR_expression: logical_AND_expression {$$ = $1;}
| logical_OR_expression LOGOR logical_AND_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::LOG_OR, $1, $3);
	}
;

logical_AND_expression: inclusive_OR_expression {$$ = $1;}
| logical_AND_expression LOGAND inclusive_OR_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::LOG_AND, $1, $3);
	}
;

inclusive_OR_expression: exclusive_OR_expression {$$ = $1;}
| inclusive_OR_expression "|" exclusive_OR_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::BIT_OR, $1, $3);
	}
;

exclusive_OR_expression: binary_and_expression {$$ = $1;}
| exclusive_OR_expression "^" binary_and_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::BIT_XOR, $1, $3);
	}
;

binary_and_expression: equality_expression {$$ = $1;}
| binary_and_expression "&" equality_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::BIT_AND, $1, $3);
	}
;

equality_expression: relational_expression {$$ = $1;}
| equality_expression EQ relational_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::EQU, $1, $3);
	}
| equality_expression NE relational_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::NEQU, $1, $3);
	}
;

relational_expression: shift_expression {$$ = $1;}
|   relational_expression "<" shift_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::LESS, $1, $3);
	}
|   relational_expression ">" shift_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::GREATER, $1, $3);
	}
|   relational_expression LE shift_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::LEQ, $1, $3);
	}
|   relational_expression GE shift_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::GEQ, $1, $3);
	}
;

shift_expression: additive_expression {$$ = $1;}
| shift_expression SHL additive_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::SHL, $1, $3);
	}
| shift_expression SHR additive_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::SHR, $1, $3);
	}
;

additive_expression: multiplicative_expression {$$ = $1;}
| additive_expression "+" multiplicative_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::ADD, $1, $3);
	}
| additive_expression "-" multiplicative_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::SUB, $1, $3);
	}
;

multiplicative_expression: unary_expression {$$ = $1;}
| multiplicative_expression "*" unary_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::MUL, $1, $3);
	}
| multiplicative_expression "/" unary_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::DIV, $1, $3);
	}
| multiplicative_expression "%" unary_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::MOD, $1, $3);
	}
;

unary_expression: postfix_expression {$$ = $1;}
| "-" unary_expression {$$ = std::make_shared<Expression>(drv.location, NodeType::MINUS, $2);}
| "!" unary_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::LOG_NOT, $2);
	}
| "~" unary_expression {
		$$ = std::make_shared<Expression>(drv.location, NodeType::BIT_NOT, $2);
	}
;

postfix_expression: primary_expression {$$ = $1;}
| postfix_expression "." id {
	$$ = std::make_shared<Expression>(drv.location, NodeType::DOT, $1, $3);
}
| postfix_expression "[" topOrEmpty "]" {
		$$ = std::make_shared<Expression>(drv.location, NodeType::ARRAY_ELEM, $1, $3);
	}
| postfix_expression "(" topOrEmpty ")" {
		$$ = std::make_shared<Expression>(drv.location, NodeType::CALL, $1, $3);
	}
 | postfix_expression "{" top "}" {
		$$ = std::make_shared<Expression>(drv.location, NodeType::OBJECT, $1, $3);
	}
;

primary_expression: id {$$ = $1;}
| I64_LITERAL {$$ = std::make_shared<Literal>(drv.location, $1);}
| F64_LITERAL {$$ = std::make_shared<Literal>(drv.location, $1);}
| CHAR_LITERAL {$$ = std::make_shared<Literal>(drv.location, $1);}
| FALSE {$$ = std::make_shared<Literal>(drv.location, false);}
| TRUE {$$ = std::make_shared<Literal>(drv.location, true);}
| STRING_LITERAL {$$ = std::make_shared<Literal>(drv.location, $1);}
| "(" top ")" {$$=$2;}
| "[" top "]" {$$ = std::make_shared<Expression>(drv.location, NodeType::MAKE_LIST, $2);}
| NIL {$$ = std::make_shared<Special>(drv.location, NodeType::NIL);}
| FAIL {$$ = std::make_shared<Special>(drv.location, NodeType::FAIL);}
;

id: IDENTIFIER {$$ = std::make_shared<ID>(drv.location, $1);}
;

topOrEmpty: top {$$=$1;}
| %empty {$$ = std::make_shared<Special>(drv.location, NodeType::EMPTY);}
;

emptyOrArgList: %empty {$$ = std::make_shared<NodeList>(drv.location, NodeType::ARGLIST);}
| argList {$$=$1;}
;

argList: id {
		$$ = std::make_shared<NodeList>(drv.location, NodeType::ARGLIST, $1);
	}
| argList "," id {$1->addNode($3); $$ = $1;}
;

%%

void yy::parser::error (const location_type& l, const std::string& m){
	std::cerr << l << ": " << m << '\n';
}

auto findElse(std::string nr, std::vector<std::vector<std::string>>& ir, unsigned i) -> unsigned{
	while(ir[i][0] != "ENDFUN"){
		if(ir[i][0] == "ELSE" && ir[i][1] == nr) return i+1;
		else ++i;
	}
	throw " internal error. No else block.";
}

auto findEndIf(std::string nr, std::vector<std::vector<std::string>>& ir, unsigned i) -> unsigned{
	while(ir[i][0] != "ENDFUN"){
		if(ir[i][0] == "ENDIF" && ir[i][1] == nr) return i;
		else ++i;
	}
	throw " internal error. No if block.";
}

auto findWhile(std::string nr, std::vector<std::vector<std::string>>& ir, unsigned i) -> unsigned{
	while(ir[i][0] != "FUN"){
		if(ir[i][0] == "WHILE" && ir[i][1] == nr) return i;
		else --i;
	}
	throw " internal error. No while block.";
}

auto findEndwhile(std::string nr, std::vector<std::vector<std::string>>& ir, unsigned i) -> unsigned{
	while(ir[i][0] != "ENDFUN"){
		if(ir[i][0] == "ENDWHILE" && ir[i][1] == nr) return i+1;
		else ++i;
	}
	throw " internal error. No while block.";
}

auto findDoWhile(std::string nr, std::vector<std::vector<std::string>>& ir, unsigned i) -> unsigned{
	while(ir[i][0] != "FUN"){
		if(ir[i][0] == "DOWHILE" && ir[i][1] == nr) return i;
		else --i;
	}
	throw " internal error. No while block.";
}

auto findNextBreaker(std::string nr, std::vector<std::vector<std::string>>& ir, unsigned i) -> unsigned{
	while(ir[i][0] != "ENDFUN"){
		if (ir[i][0] == "ENDDOWHILE" && ir[i][1] == nr) return i + 1;
        else if (ir[i][0] == "ENDWHILE" && ir[i][1] == nr) return i;
        else if (ir[i][0] == "ENDEVERY" && ir[i][1] == nr) return i;
        else ++i;
	}
	throw "break outside loop.";
}

auto findNextContinue(std::string nr, std::vector<std::vector<std::string>>& ir, unsigned i) -> unsigned{
	while(ir[i][0] != "ENDFUN"){
		if (ir[i][0] == "ENDDOWHILE" && ir[i][1] == nr) return i + 1;
        else if (ir[i][0] == "ENDWHILE" && ir[i][1] == nr) return i;
        else ++i;
	}
	throw "break outside loop.";
}

auto findEndEvery(std::string nr, std::vector<std::vector<std::string>>& ir, unsigned i){
    while(ir[i][0] != "ENDFUN"){
        if(ir[i][0] == "ENDEVERY" && ir[i][1] == nr) return i;
        else ++i;
	}
    throw "operator and outside every block.";
}

auto calculateJumps(std::vector<std::vector<std::string>> ir) -> std::vector<std::vector<std::string>>{
	unsigned i = 0;
    for(auto& a : ir){
		if(a.size() > 0 && a[0] == "THEN") a.push_back(std::to_string(findElse(a[1], ir, i)));
        if(a.size() > 0 && a[0] == "ELSE") a.push_back(std::to_string(findEndIf(a[1], ir, i)));
        if(a.size() > 0 && a[0] == "DO") a.push_back(std::to_string(findEndwhile(a[1], ir, i)));
        if(a.size() > 0 && a[0] == "ENDWHILE") a.push_back(std::to_string(findWhile(a[1], ir, i)));
        if(a.size() > 0 && a[0] == "ENDDOWHILE") a.push_back(std::to_string(findDoWhile(a[1], ir, i)));
        if(a.size() > 0 && (a[0] == "EVERYBODY" || a[0] == "NDT_AND")) a.push_back(std::to_string(findEndEvery(a[1], ir, i)));
        if(a.size() > 0 && a[0] == "BREAK") a.push_back(std::to_string(findNextBreaker(a[1], ir, i)));
        if(a.size() > 0 && a[0] == "CONTINUE") a.push_back(std::to_string(findNextContinue(a[1], ir, i)));
        ++i;
	}
    return ir;
}

std::vector<std::string> split(const std::string& line, char delimiter){
    std::vector <std::string> tokens;
    std::stringstream check1(line);
    std::string intermediate;
    while(getline(check1, intermediate, delimiter)){ 
        tokens.push_back(intermediate); 
    }
    return tokens; 
}

std::vector<std::vector<std::string>> generateIR(std::shared_ptr<NodeList> ast){
	std::vector<std::vector<std::string>> ir;
	std::string ir_string(ast->generatr_ir());
	auto temp = split(ir_string, '\n');
	for(auto& a : temp) ir.push_back(split(a, ' '));
	return ir;
}

std::shared_ptr<NodeList> semantic(std::shared_ptr<NodeList> ast){
    ast->semantic(SemanticAnalyzerHelper(0));
	return ast;
}

std::vector<std::vector<std::string>> parserMain(std::shared_ptr<NodeList> programList){
    return calculateJumps(generateIR(semantic(programList)));
}

extern void interpreterMain(std::vector<std::vector<std::string>> ir, bool d);
int main (int argc, char *argv[]){
	int res = 0;
	driver drv;
	for (int i = 1; i < argc; ++i)
		if (argv[i] == std::string ("-d")){drv.debug = true;}
		else if(!drv.parse(argv[i])){}
		else{res = 1;}
	if(res == 0){
		std::vector<std::vector<std::string>> ir;
		try{
			ir = parserMain(programList);
		} catch(std::string err) {
			std::cout<< "Semantic error. " << err <<'\n';
			exit(1);
		}
		if(drv.debug){
			std::cout<<"======== CODE ========\n";
			auto i = 0;
			for(auto& a : ir){
				std::cout<<i<<" ";
				for(auto& b : a){
					std::cout<< b <<" ";
				}
				std::cout<<'\n';
				++i;
			}
		}
		interpreterMain(ir, drv.debug);
	} else {
		std::cerr << "Wrong arguments\n";
	}
	return 0;
}