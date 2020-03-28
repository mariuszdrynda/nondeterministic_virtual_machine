#include "ast.hpp"
#include <string>

Literal::Literal(){}
std::string Literal::print(){
    return "(LITERAL)";
}
ID::ID(){}
std::string ID::print(){
    return "(ID)";
}