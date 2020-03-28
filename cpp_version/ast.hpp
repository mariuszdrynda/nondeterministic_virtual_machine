#pragma once
#include <string>
#include <variant>

struct AST{
    virtual std::string print() = 0;
};
struct Literal : AST{
    //template<typename T> Literal(T val);
    Literal(char val);
    Literal(long long val);
    Literal(bool val);
    Literal(double val);
    std::string print();
private:
    std::variant<long long, char, bool, double> value;
};
struct ID : AST{
    ID(std::string n);
    std::string print();
private:
    std::string name;
};