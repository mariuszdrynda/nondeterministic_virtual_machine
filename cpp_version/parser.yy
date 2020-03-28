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
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
;
%type  <std::shared_ptr<AST>> primary_expression
%token <std::string> IDENTIFIER
%token <double> F64_LITERAL
%token <long long> I64_LITERAL
%token <char> CHAR_LITERAL
%token <bool> TRUE FALSE

%%

%start top;

top: primary_expression { std::cout << $1->print(); }

primary_expression: IDENTIFIER { $$ = std::make_shared<ID>($1); }
| I64_LITERAL {$$ = std::make_shared<Literal>($1);}
| F64_LITERAL {$$ = std::make_shared<Literal>($1);}
| CHAR_LITERAL {$$ = std::make_shared<Literal>($1);}
| FALSE {$$ = std::make_shared<Literal>(false);}
| TRUE {$$ = std::make_shared<Literal>(true);}
;

// primary_expression: 
// | NIL {$$=new Node(yylineno, "NIL");}
// | lambda {$$=$1;}
// | STRING_LITERAL {$$ = new Node(yylineno, "STRING", $1);}
// | SUCCESS {$$=new Node(yylineno, "SUCCESS");}
// | FAIL {$$=new Node(yylineno, "FAIL");}
// | NOP {$$=new Node(yylineno, "NOP");}
// | '_' {$$=new Node(yylineno, "UNDERSCORE");}
// | '(' top ')' {$$=$2;}
// | '{' topList '}' {$$ = new Node(yylineno, "OBJECT", $2);}
// | '[' topList ']' {$$ = new Node(yylineno, "LIST", $2);}
// ;

%%

void yy::parser::error (const location_type& l, const std::string& m){
	std::cerr << l << ": " << m << '\n';
}

int main (int argc, char *argv[]){
	driver drv;
	drv.parse(argv[1]);
    std::cout<<"\tPARSE DONE!\n";
    // cout<<(programList->printList())<<'\n';
	return 0;
}