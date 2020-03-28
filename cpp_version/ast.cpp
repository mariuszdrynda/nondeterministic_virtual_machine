#include "ast.hpp"
#include <string>

// template<typename T> Literal::Literal(T val) : value(val){}
Literal::Literal(char val): value(val){}
Literal::Literal(long long val): value(val){}
Literal::Literal(bool val): value(val){}
Literal::Literal(double val): value(val){}
std::string Literal::print(){
    // const auto a = std::get<1>(value);
    return "(LITERAL("+std::to_string(value.index())+"))";
}
ID::ID(std::string n): name(n){}
std::string ID::print(){
    return "(ID("+name+"))";
}