# Nondeterministic Virtual Machine

This will be a virtual machine for nondeterministic programming language write in C++ (also using Flex & Bison). Project will be divided into two parts: first, compiler of nondeterministic programming language, and second: virtual machine.

Compiler will generate abstract syntax tree from input code and pass it into VM. VM will execute code.

Current status: compiler generates syntax tree, but still need to fix a few bugs. Then need to implement semantic analyzer. Then perhaps do some optimalizations.

Virtual machine doesn't exists yet.

Requirements: Bison 3.4, FLex 2.6.4, C++ 14 (I'm using G++ 7.2)

### Usage

- bison -d parser.y
- flex lexer.l
- g++ -std=c++14 parser.tab.c lex.yy.c node.cpp

## WORK THAT REMAINS

### PARSER

- add modules
- add escape sequences in chars and strings
- write own error messages (instead of default "syntax error")
- change type of name string in node and nodeList to enum
- Freeing Discarded Symbols (chapter 3.7.7 in bison's doc)

##### and fix bugs

- IDs contains too much information
- compilation is not terminating after seing unrecognized token

### SEMANTIC ANALYZER

- build proper functions from generics functions
- build proper structs from generic structures
- matching functions to their calls

##### Detection of errors

- missing '_' in switch
- a = 5 = 7
- undefined operation for variables of two different types (example: a+b, for a:Int, b:String)
- divisoin by 0
- index out of bound
- two functions of same arguments types, and return type

### OPTIMALIZATIONS

- counting of constexpr functions
- deleting never used functions
- deleting never user structures (or fields)

### VIRTUAL MACHINE

- scheduler
- executor
- garbage collector (or whatever to deal with memory management)

### OTHERS

- add example codes
- write some simple libraries

## PROPOSITIONS

- delete else keyword