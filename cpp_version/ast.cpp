#include "ast.hpp"
#include <string>
#include "location.hh"

Special::Special(SpecialType t): type(t){}
std::string Special::print(){
    return "(SPECIAL("+std::to_string(type)+"))";
}
// template<typename T> Literal::Literal(T val) : value(val){}
Literal::Literal(yy::location loc, char val): value(val){
    AST::location = loc;
}
Literal::Literal(yy::location loc, long long val): value(val){
    AST::location = loc;
}
Literal::Literal(yy::location loc, bool val): value(val){
    AST::location = loc;
}
Literal::Literal(yy::location loc, double val): value(val){
    AST::location = loc;
}
Literal::Literal(yy::location loc, std::string val): value(val){
    AST::location = loc;
}
std::string Literal::print(){
    // const auto a = std::get<1>(value);
    return "(LITERAL["+std::to_string(AST::location.begin.line)+"]"+std::to_string(value.index())+"))";
}
ID::ID(yy::location loc, std::string n): name(n){
    AST::location = loc;
}
std::string ID::print(){
    return "(ID["+std::to_string(AST::location.begin.line)+"]"+name+")";
}
Object::Object(yy::location loc, std::shared_ptr<AST> v): value(v){
    AST::location = loc;
}
std::string Object::print(){
    return "(OBJECT["+
        std::to_string(AST::location.begin.line)+
        "]"+value->print()+")";
}
List::List(yy::location loc, std::shared_ptr<AST> v): value(v){
    AST::location = loc;
}
std::string List::print(){
    return "(LIST["+
        std::to_string(AST::location.begin.line)+
        "]"+value->print()+")";
}