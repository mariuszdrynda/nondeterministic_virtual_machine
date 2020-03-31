#pragma once
#include <string>
#include <variant>
#include <memory>
#include <vector>
#include "location.hh"

enum StatementType{
    SWITCH, CASE, EVERY, WHILE, DO_WHILE, IF
};
enum ExpressionType{
    ASS, NDT_AND, NDT_OR, LIMIT, NDT_NOT, LOG_OR, LOG_AND, BIT_OR, BIT_XOR, 
    BIT_AND, EQU, NEQU, LESS, GREATER, LEQ, GEQ, SHL, SHR, ADD, SUB, MUL,
    DIV, MOD, MINUS, LOG_NOT, BIT_NOT, DOT, ARRAY_ELEM, CALL
};
enum SpecialType{
    CONTINUE, BREAK, NIL, SUCCESS, FAIL, NOP, UNDERSCORE, EMPTY
};
enum NodeListType{
    SEPARATOR, CASE_LIST, COMMA
};
enum ReturnType{
    RET, YIELD
};
struct AST{
    virtual std::string print() = 0;
    yy::location location;
};
struct Return : AST{
    Return(yy::location loc, ReturnType t, std::shared_ptr<AST> e);
    std::string print();
private:
    ReturnType returnType;
    std::shared_ptr<AST> expression;
};
struct Statement : AST{
    Statement(yy::location loc, StatementType type, std::shared_ptr<AST> c, std::shared_ptr<AST> t, std::shared_ptr<AST> e);
    Statement(yy::location loc, StatementType type, std::shared_ptr<AST> c, std::shared_ptr<AST> t);
    std::string print();
private:
    StatementType statementType;
    std::shared_ptr<AST> condition;
    std::shared_ptr<AST> body;
    std::shared_ptr<AST> elseBody;
};
struct NodeList : AST{
    NodeList(yy::location loc, NodeListType t, std::shared_ptr<AST> e);
    std::string print();
    void addNode(std::shared_ptr<AST> node);
    bool hasOneElement();
    std::shared_ptr<AST> giveMeOnlyElem();
private:
    std::vector<std::shared_ptr<AST>> list;
    NodeListType type;
};
struct Expression : AST{
    Expression(yy::location loc, ExpressionType t, std::shared_ptr<AST> l, std::shared_ptr<AST> r);
    Expression(yy::location loc, ExpressionType t, std::shared_ptr<AST> l);
    std::string print();
private:
    ExpressionType expressionType;
    std::shared_ptr<AST> left;
    std::shared_ptr<AST> right;
};
struct Special : AST{
    Special(yy::location loc, SpecialType t);
    std::string print();
private:
    SpecialType type;
};
struct Literal : AST{
    //template<typename T> Literal(T val);
    Literal(yy::location loc, char val);
    Literal(yy::location loc, long long val);
    Literal(yy::location loc, bool val);
    Literal(yy::location loc, double val);
    Literal(yy::location loc, std::string val);
    std::string print();
private:
    std::variant<long long, char, bool, double, std::string> value;
};
struct ID : AST{
    ID(yy::location loc, std::string n);
    std::string print();
private:
    std::string name;
};
struct Object : AST{
    Object(yy::location loc, std::shared_ptr<AST> v);
    std::string print();
private:
    std::shared_ptr<AST> value;
};
struct List : AST{
    List(yy::location loc, std::shared_ptr<AST> v);
    std::string print();
private:
    std::shared_ptr<AST> value;
};