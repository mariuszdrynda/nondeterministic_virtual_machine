#pragma once
#include <string>
#include <variant>
#include <memory>
#include "location.hh"

enum ExpressionType{
    BIT_OR, BIT_XOR, BIT_AND, EQU, NEQU,
    LESS, GREATER, LEQ, GEQ, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
    MINUS, LOG_NOT, BIT_NOT, DOT, ARRAY_ELEM, CALL
};
enum SpecialType{
    NIL, SUCCESS, FAIL, NOP, UNDERSCORE, EMPTY
};
struct AST{
    virtual std::string print() = 0;
    yy::location location;
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
    Special(SpecialType t);
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