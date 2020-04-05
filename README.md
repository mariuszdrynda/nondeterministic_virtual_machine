# Nondeterministic Virtual Machine

This will be a virtual machine for nondeterministic programming language written in C++ (with help of Flex & Bison). This project will be divided into two parts: first, compiler of nondeterministic programming language, and the second one: code executor.

Current status: compiler generates syntax tree, but still need to fix a few bugs. Then need to implement semantic analyzer. Then perhaps do some optimalizations.

Executor doesn't exists yet.

If you want to compile this project you need Bison 3.4, Flex 2.6.4 and C++ 17 (I'm using G++ 7.2). You can use any later version of Bison and/or flex. C++ 17 is required, because AST uses std::variant.

### Usage

- bison -d parser.yy
- flex scanner.l
- g++ -std=c++17 driver.cc parser.tab.cc lex.yy.c ast.cpp semanticAnalyzer.cpp

## SEMANTIC ANALYZER

##### Detection of errors

- no main function
- more than one function 
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

- build proper functions from generics functions
- build proper structs from generic structures
- matching functions to their calls

##### Detection of errors

- function main argument type other than List of String
- calling function on left side of assignment
- expression on left side of assignment
- literal on left side of assignment (example: a = 5 = 7)
- first occurence of identifier on right side of assignment expression
- first occurence of identifier outside assignment expression
- break / continue outside loop
- other than 2 args in +/-/*/% operator overloading
- no field in object (example: struct A{a:I64} /**/ obj = A(0);obj.b)
- missing '_' in switch
- undefined operation for variables of two different types (example: a+b, for a:Int, b:String)
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

##### Catching runtime errors

- division by 0
- index out of bound

### OTHERS

- add example codes
- write some simple libraries

## FUTURE

- debugger
- doc generator

## PROPOSITIONS

- add ranges
- : as type assignment operator
- add User-defined literals
- add laziness
- add modules
- add shell