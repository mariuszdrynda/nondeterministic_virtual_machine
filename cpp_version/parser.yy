%skeleton "lalr1.cc"
%require "3.4"
%defines
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
	#include <string>
	class driver;
}

%param { driver& drv }
%locations
%define parse.error verbose

%code {
	#include "driver.hh"
	yy::parser::symbol_type yylex(driver& drv);
	extern FILE *yyin;
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
;
%type  <long> exp
%token <std::string> IDENTIFIER
%token <long> I64LITERAL

%%

%start top;

top: exp { std::cout << $1; }

exp: IDENTIFIER { $$ = 5; }
| I64LITERAL { $$=$1; }
;

%%

void yy::parser::error (const location_type& l, const std::string& m){
	std::cerr << l << ": " << m << '\n';
}

int main (int argc, char *argv[]){
	driver drv;
	drv.parse(argv[1]);
	return 0;
}