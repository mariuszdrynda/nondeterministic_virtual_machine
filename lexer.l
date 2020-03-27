%{
    #include <iostream>
    #include "node.hpp"
    #include "parser.tab.h"
    using namespace std;
%}

%option noyywrap yylineno

fractional_constant (({digit_sequence}?"."{digit_sequence})|({digit_sequence}"."))
exponent_part ([Ee][+-]?{digit_sequence})
digit_sequence ([0-9]("\'"?[0-9])*)
s_char [^"\"""\\"\n]
c_char [^"\'""\\"\n]

%%

    /*TYPES*/

"String"    {return STRING;}
"I64"   {return I64;}
"F64"   {return F64;}
"Char"  {return CHAR;}
"Bool"  {return BOOL;}
"Void"  {return VOID;}

    /*keywords*/

"operator"  {return OPERATOR;}
"continue"  {return CONTINUE;}
"success"   {return SUCCESS;}
"switch"    {return SWITCH;}
"struct"    {return STRUCT;}
"data"  {return DATA;}
"fn"    {return FN;}
"lazy"  {return LAZY;}
"ret"   {return RET;}
"yield" {return YIELD;}
"case"  {return CASE;}
"every" {return EVERY;}
"while" {return WHILE;}
"do"    {return DO;}
"if"    {return IF;}
"else"  {return ELSE;}
"break" {return BREAK;}
"nop"   {return NOP;}
"nil"   {return NIL;}
"self"  {return SELF;}
"fail"  {return FAIL;}

    /*operators*/

"limit" {return LIMIT;}
"or"    {return NDTOR;}
"and"   {return NDTAND;}
"not"   {return NDTNOT;}
"\\\\"  {return LAMBDA_BEGINER;}
"->"    {return ARROW;}
"=="    {return EQ;}
"!="    {return NEQ;}
">="    {return GE;}
"<="    {return LE;}
"<<"    {return SHL;}
">>"    {return SHR;}
"||"    {return LOGOR;}
"&&"    {return LOGAND;}
"=" {return '=';}
"," {return ',';}
"|" {return '|';}
"^" {return '^';}
"&" {return '&';}
"<" {return '<';}
">" {return '>';}
"+" {return '+';}
"*" {return '*';}
"/" {return '/';}
"%" {return '%';}
"-" {return '-';}
"!" {return '!';} 
"~" {return '~';}
"." {return '.';}
":" {return ':';}
"_" {return '_';}
";" {return ';';}
"}" {return '}';}
"[" {return '[';}
"]" {return ']';}
")" {return ')';}
"{" {return '{';}
"(" {return '(';}

    /*literals*/

"false" {return FALSE;}
"true" {return TRUE;}
[1-9]("\'"?[0-9])*("i64")? {yylval.I64_LITERAL = stoll(yytext); return I64_LITERAL;}
(({fractional_constant}{exponent_part}?)|({digit_sequence}{exponent_part}))("f64")? {yylval.F64_LITERAL = stof(yytext); return F64_LITERAL;}
"\'"{c_char}"\'" {yylval.CHAR_LITERAL = yytext[1]; return CHAR_LITERAL;}
"\""({s_char})*"\"" {yylval.STRING_LITERAL = yytext; return STRING_LITERAL;}

    /* identifier */

[_a-zA-Z][_a-zA-Z0-9]* {yylval.ID = yytext; return ID;}

    /*spaces*/

[ \t]   {/*do nothing*/}
\n      {/*add newline*/}
.       {cerr<<"Error. Unrecognized token\n";}

    /*comments*/

"//"([^\n\\]|"\\\n")*$ {/*do nothing*/} /* TODO błąd */
"/*"([^"*"]|("*"[^"/"]))*"*/" {/*do nothing*/}

%%