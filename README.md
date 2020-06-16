# Nondeterministic Virtual Machine

This is a virtual machine for nondeterministic programming language (NJA) written in C++ (with help of Flex & Bison).

If you want to compile this project you need Bison 3.4, Flex 2.6.4 and C++ 17 (I'm using G++ 7.2). You can use any later version of Bison and/or flex. C++ 17 is required, because AST uses std::variant.

### Usage

- bison -d parser.yy
- flex scanner.l
- g++ -std=c++17 driver.cc parser.tab.cc lex.yy.c ast/ast.cpp interpreter.cpp -o vm

## WORK THAT REMAINS

### PARSER

- add escape sequences in chars and strings
- write own error messages (instead of default "syntax error")
- Freeing Discarded Symbols (chapter 3.7.7 in bison's doc)

##### and fix bugs

- comment // in last line of source code throws "EOF expected" error

### SEMANTIC ANALYZER

##### Detection of errors

- first occurence of identifier on right side of assignment expression
- first occurence of identifier outside assignment expression

### OPTIMALIZATIONS

- count of constexpr functions
- deleting never used functions
- deleting never user structures (or fields)
- cut everything from ret to funcion's end

## PROPOSITIONS

- add more build-in functions
- switch / match
- types
- function overriding
- lambda / clojures
- ranges
- : as type assignment operator
- User-defined literals
- laziness
- modules
- shell
- operator overloading

## FUTURE

- debugger
- doc generator