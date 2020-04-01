# Nondeterministic Virtual Machine

This will be a virtual machine for nondeterministic programming language write in C++ (also using Flex & Bison). Project will be divided into two parts: first, compiler of nondeterministic programming language, and second: virtual machine.

Compiler will generate abstract syntax tree from input code and pass it into VM. VM will execute code.

Current status: compiler generates syntax tree, but still need to fix a few bugs. Then need to implement semantic analyzer. Then perhaps do some optimalizations.

Virtual machine doesn't exists yet.

Requirements: Bison 3.4, Flex 2.6.4, C++ 17 (I'm using G++ 7.2). C++ 17 is required, because AST uses std::variant.

### Usage

- bison -d parser.yy
- flex scanner.l
- g++ -std=c++17 driver.cc parser.tab.cc lex.yy.c ast.cpp

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

- build proper functions from generics functions
- build proper structs from generic structures
- matching functions to their calls

##### Detection of errors

- missing '_' in switch
- a = 5 = 7
- undefined operation for variables of two different types (example: a+b, for a:Int, b:String)
- division by 0
- index out of bound
- two functions of same arguments types, and return type

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

- add laziness
- add modules
- add shell