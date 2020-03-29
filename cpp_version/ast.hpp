#pragma once
#include <string>
#include <variant>
#include <memory>
#include "location.hh"

enum SpecialType{
    NIL, SUCCESS, FAIL, NOP, UNDERSCORE, EMPTY
};
struct AST{
    virtual std::string print() = 0;
    yy::location location;
};
struct Special : AST{
    Special(SpecialType t);
    SpecialType type;
    std::string print();
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