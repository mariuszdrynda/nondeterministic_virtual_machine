Nondeterministic Virtual Machine

This will be virtual machine for nondeterministic programming language write in C++ (also using Flex & Bison). Project will be divided into two parts: first, compiler of nondeterministic programming language, and second: virtual machine.

Compiler will generate absstract syntax tree from input code and pass it into VM. VM will 

Curresnt status: compiler generates syntax tree, but still need to fix a few bugs. Then need to implement semantic analyzer. Then perhaps do some optimalizations.

Virtual machine doesn't exists yet.

Requirements: Bison 3.4, FLex 2.6.4, C++ 17

Usage:

bison -d parser.y
flex lexer.l
g++ -std=c++17 parser.tab.c lex.yy.c node.cpp

WORK THAT REMAINS:

PARSER

write own error messages (instead of default "syntax error")
add string literal
add char literal
fix bug: IDs contains too much information
change type of name string in node and nodeList to enum
tests comments
Freeing Discarded Symbols (chapter 3.7.7 in bison doc)

SEMANTIC ANALYZER

...

VIRTUAL MACHINE

...