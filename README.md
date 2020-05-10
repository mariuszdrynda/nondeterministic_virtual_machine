# Nondeterministic Virtual Machine

This will be a virtual machine for nondeterministic programming language (NJA) written in C++ (with help of Flex & Bison). This project will be divided into two parts: first, compiler of nondeterministic programming language, and the second one: code executor.

Current status od C++ version: compiler generates syntax tree, but still need to fix a few bugs. Then need to implement semantic analyzer. Then perhaps do some optimalizations.

Executor doesn't exists yet.

Python version is an experimental implementation of NJA. I chose it because I wanted to have a playground where I can put all of my ideas and check if they work. It's not for normal usage. It's a dirty version and it not checks all errors and exceptions, so use it only if you know what to do.

Current status of Python version: compiler and interpreter works but need to be tested.

Python version needs PLY to work.

If you want to compile this project you need Bison 3.4, Flex 2.6.4 and C++ 17 (I'm using G++ 7.2). You can use any later version of Bison and/or flex. C++ 17 is required, because AST uses std::variant.

### Usage

- bison -d parser.yy
- flex scanner.l
- g++ -std=c++17 driver.cc parser.tab.cc lex.yy.c ast.cpp semanticAnalyzer.cpp

## SEMANTIC ANALYZER

##### Detection of errors

- more than one argument for function main

## WORK THAT REMAINS

### PARSER

- add escape sequences in chars and strings
- write own error messages (instead of default "syntax error")
- Freeing Discarded Symbols (chapter 3.7.7 in bison's doc)

##### and fix bugs

- comment // in last line of source code throws "EOF expected" error
- not counting lines when /**/ comment is using
- compilation is not terminating after seing unrecognized token

### SEMANTIC ANALYZER

##### Detection of errors

- calling function on left side of assignment
- calling for not defined function
- expression on left side of assignment
- literal on left side of assignment (example: a = 5 = 7)
- first occurence of identifier on right side of assignment expression
- first occurence of identifier outside assignment expression
- break / continue outside loop
- no field in object (example: struct A{a:I64} /**/ obj = A(0); obj.b)

### OPTIMALIZATIONS

- count of constexpr functions
- deleting never used functions
- deleting never user structures (or fields)
- cut everything from ret to funcion's end

### VIRTUAL MACHINE

- scheduler
- executor
- garbage collector (or whatever to deal with memory management)

### OTHERS

- add example codes
- write some simple libraries

## FUTURE

- debugger
- doc generator

## PROPOSITIONS

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