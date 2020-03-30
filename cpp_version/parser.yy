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
	std::shared_ptr<AST> programList;
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
  UNDERSCORE "_"
;
%type  <std::shared_ptr<AST>> topList top 
%type  <std::shared_ptr<AST>> inclusive_OR_expression exclusive_OR_expression binary_and_expression equality_expression
%type  <std::shared_ptr<AST>> relational_expression shift_expression additive_expression multiplicative_expression 
%type  <std::shared_ptr<AST>> unary_expression postfix_expression primary_expression lambda topOrEmpty
%type  <std::shared_ptr<ID>> id
%token <std::string> IDENTIFIER STRING_LITERAL
%token <double> F64_LITERAL
%token <long long> I64_LITERAL
%token <char> CHAR_LITERAL
%token <bool> TRUE FALSE
%token EQ NE LE GE SHL SHR NIL SUCCESS FAIL NOP LAMBDA_BEGINER

%%

%start program;

program: topList {programList = $1;}
;

topList: top {$$=$1;}
;

top: inclusive_OR_expression {$$=$1;;}
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
 // | postfix_expression '{' topOrEmpty '}'
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
| NIL {$$ = std::make_shared<Special>(SpecialType::NIL);}
| SUCCESS {$$ = std::make_shared<Special>(SpecialType::SUCCESS);}
| FAIL {$$ = std::make_shared<Special>(SpecialType::FAIL);}
| NOP {$$ = std::make_shared<Special>(SpecialType::NOP);}
| "_" {$$ = std::make_shared<Special>(SpecialType::UNDERSCORE);}
| lambda {$$=$1;}
;

id: IDENTIFIER {$$ = std::make_shared<ID>(drv.location, $1);}
;

topOrEmpty: top {$$=$1;}
| %empty {$$ = std::make_shared<Special>(SpecialType::EMPTY);}
;

lambda: LAMBDA_BEGINER {$$ = std::make_shared<Special>(SpecialType::EMPTY);} //TODO
;

%%

void yy::parser::error (const location_type& l, const std::string& m){
	std::cerr << l << ": " << m << '\n';
}

int main (int argc, char *argv[]){
	driver drv;
	drv.parse(argv[1]);
    std::cout<<"\tPARSE DONE!\n";
    std::cout<<(programList->print())<<'\n';
	return 0;
}