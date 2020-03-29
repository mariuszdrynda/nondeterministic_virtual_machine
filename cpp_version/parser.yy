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
  UNDERSCORE "_"
;
%type  <std::shared_ptr<AST>> topList top primary_expression lambda
%token <std::string> IDENTIFIER STRING_LITERAL
%token <double> F64_LITERAL
%token <long long> I64_LITERAL
%token <char> CHAR_LITERAL
%token <bool> TRUE FALSE
%token NIL SUCCESS FAIL NOP

%%

%start program;

program: topList {programList = $1;}

topList: top {$$=$1;}

top: primary_expression {$$=$1;}

primary_expression: IDENTIFIER { $$ = std::make_shared<ID>(drv.location, $1); }
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

lambda: %empty {$$ = std::make_shared<Special>(SpecialType::EMPTY);} //TODO
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