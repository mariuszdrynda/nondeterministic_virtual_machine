#pragma once
#include <string>

struct AST{
    virtual std::string print() = 0;
};
struct Literal : AST{
    Literal();
    std::string print();
};
struct ID : AST{
    ID();
    std::string print();
};