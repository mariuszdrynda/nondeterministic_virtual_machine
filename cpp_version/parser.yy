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
%type  <std::shared_ptr<AST>> topList top additive_expression multiplicative_expression
%type  <std::shared_ptr<AST>> unary_expression postfix_expression primary_expression lambda topOrEmpty
%type  <std::shared_ptr<ID>> id
%token <std::string> IDENTIFIER STRING_LITERAL
%token <double> F64_LITERAL
%token <long long> I64_LITERAL
%token <char> CHAR_LITERAL
%token <bool> TRUE FALSE
%token NIL SUCCESS FAIL NOP LAMBDA_BEGINER

%%

%start program;

program: topList {programList = $1;}
;

topList: top {$$=$1;}
;

top: additive_expression {$$=$1;;}
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