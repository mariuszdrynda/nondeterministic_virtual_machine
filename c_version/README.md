This is old version of parser written in C++, but using C interface of Bison.

### Usage

- bison -d parser.y
- flex lexer.l
- g++ -std=c++17 parser.tab.c lex.yy.c node.cpp