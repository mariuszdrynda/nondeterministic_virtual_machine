#include "ast.hpp"
#include <string>
#include "location.hh"

Return::Return(yy::location loc, ReturnType t, std::shared_ptr<AST> e): returnType(t), expression(e){
    AST::location = loc;
}
std::string Return::print(){
    return "(RETURN"+std::to_string(returnType)+expression->print()+")";
}
Statement::Statement(yy::location loc, StatementType type, std::shared_ptr<AST> c, std::shared_ptr<AST> t, std::shared_ptr<AST> e)
    :statementType(type), condition(c), body(t), elseBody(e){
    AST::location = loc;
}
Statement::Statement(yy::location loc, StatementType type, std::shared_ptr<AST> c, std::shared_ptr<AST> t)
    :statementType(type), condition(c), body(t){
    AST::location = loc;
}
std::string Statement::print(){
    return "(STATEMENT"+std::to_string(statementType)+
    condition->print()+
    body->print()+(elseBody ? elseBody->print() : "")+")";
}
NodeList::NodeList(yy::location loc, NodeListType t, std::shared_ptr<AST> e): type(t){
    AST::location = loc;
    list.push_back(e);
}
std::string NodeList::print(){
    if(list.size() == 1) return list[0]->print();
    std::string result = "(NODELIST"+std::to_string(type)+"["+std::to_string(AST::location.begin.line)+"]";
    for(auto listElem : list) result += listElem->print();
    return result+")";
}
void NodeList::addNode(std::shared_ptr<AST> node){
    list.push_back(node);
}
bool NodeList::hasOneElement(){
    return list.size() == 1;
}
std::shared_ptr<AST> NodeList::giveMeOnlyElem(){
    return list[0];
}
Expression::Expression(yy::location loc, ExpressionType t, std::shared_ptr<AST> l, std::shared_ptr<AST> r)
    : expressionType(t), left(l), right(r){
        AST::location = loc;
    }
Expression::Expression(yy::location loc, ExpressionType t, std::shared_ptr<AST> l): expressionType(t), left(l){
        AST::location = loc;
    }
std::string Expression::print(){
    if(right) return "(EXPRESSION"+std::to_string(expressionType)+left->print()+right->print()+")";
    else return "(EXPRESSION"+std::to_string(expressionType)+left->print()+")";
}
Special::Special(yy::location loc, SpecialType t): type(t){
    AST::location = loc;
}
std::string Special::print(){
    return "(SPECIAL"+std::to_string(type)+")";
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
    return "(LITERAL["+std::to_string(AST::location.begin.line)+"]"+std::to_string(value.index())+")";
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