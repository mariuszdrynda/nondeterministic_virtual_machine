%{
	#include <cerrno>
	#include <climits>
	#include <cstdlib>
	#include <cstring> // strerror
	#include <string>
	#include "driver.hh"
    #include "ast.hpp"
	#include "parser.tab.hh"
	#define YY_DECL yy::parser::symbol_type yylex(driver& drv)
%}

%option noyywrap nounput noinput batch

%{
  yy::parser::symbol_type make_I64_LITERAL(const std::string &s, const yy::parser::location_type& loc);
  yy::parser::symbol_type make_F64_LITERAL(const std::string &s, const yy::parser::location_type& loc);
  yy::parser::symbol_type make_CHAR_LITERAL(const std::string &s, const yy::parser::location_type& loc);
  yy::parser::symbol_type make_STRING_LITERAL(const std::string &s, const yy::parser::location_type& loc);
%}

id    [a-zA-Z][a-zA-Z_0-9]*
fractional_constant (({digit_sequence}?"."{digit_sequence})|({digit_sequence}"."))
exponent_part ([Ee][+-]?{digit_sequence})
digit_sequence ([0-9]("\'"?[0-9])*)
int   [1-9][0-9]+
blank [ \t\r]
s_char [^"\"""\\"\n]
c_char [^"\'""\\"\n]

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
%}

%%

%{
  // A handy shortcut to the location held by the driver.
  yy::location& loc = drv.location;
  // Code run each time yylex is called.
  loc.step();
%}

    /* TYPES */
	/* keywords */

"success"	return yy::parser::make_SUCCESS(loc);
"nop"   	return yy::parser::make_NOP(loc);
"nil"   	return yy::parser::make_NIL(loc);
"fail"		return yy::parser::make_FAIL(loc);

	/* operators */

"("     return yy::parser::make_LPAREN(loc);
")"     return yy::parser::make_RPAREN(loc);
"{"     return yy::parser::make_LCB(loc);
"}"     return yy::parser::make_RCB(loc);
"["     return yy::parser::make_LSB(loc);
"]"     return yy::parser::make_RSB(loc);
"_"		return yy::parser::make_UNDERSCORE(loc);

	/* LITERALS */

"true"		return yy::parser::make_TRUE(yytext, loc);
"false"		return yy::parser::make_FALSE(yytext, loc);
{int}		return make_I64_LITERAL(yytext, loc);
(({fractional_constant}{exponent_part}?)|({digit_sequence}{exponent_part}))("f64")? return make_F64_LITERAL(yytext, loc);
"\'"{c_char}"\'"	return make_CHAR_LITERAL(yytext, loc);
"\""({s_char})*"\"" return make_STRING_LITERAL(yytext, loc);

    /* identifier */

{id}       return yy::parser::make_IDENTIFIER(yytext, loc);

    /* comments */

    /* spaces */

{blank}+   loc.step();
\n+        loc.lines(yyleng); loc.step();
.	{throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext));}

<<EOF>>    return yy::parser::make_END(loc);

%%

yy::parser::symbol_type make_CHAR_LITERAL(const std::string &s, const yy::parser::location_type& loc){
	return yy::parser::make_CHAR_LITERAL((char) yytext[1], loc);
}
yy::parser::symbol_type make_STRING_LITERAL(const std::string &s, const yy::parser::location_type& loc){
	return yy::parser::make_STRING_LITERAL(yytext, loc);
}
yy::parser::symbol_type make_F64_LITERAL(const std::string &s, const yy::parser::location_type& loc){
	errno = 0;
	double n = std::stod(s.c_str());
	return yy::parser::make_F64_LITERAL((double) 0, loc);
}
yy::parser::symbol_type make_I64_LITERAL(const std::string &s, const yy::parser::location_type& loc){
	errno = 0;
	long long n = strtoll(s.c_str(), NULL, 10);
	if (! (LLONG_MIN <= n && n <= LLONG_MAX && errno != ERANGE))
		throw yy::parser::syntax_error (loc, "i64 is out of range: " + s);
	return yy::parser::make_I64_LITERAL((long long) n, loc);
}

void driver::scan_begin(){
	if (file.empty() || file == "-") yyin = stdin;
	else if (!(yyin = fopen (file.c_str(), "r"))){
		std::cerr << "cannot open " << file << ": " << strerror(errno) << '\n';
		exit (EXIT_FAILURE);
	}
}

void driver::scan_end(){
	fclose (yyin);
}