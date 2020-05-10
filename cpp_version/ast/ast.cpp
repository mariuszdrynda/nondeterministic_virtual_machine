#include "ast.hpp"
#include <string>
#include "../location.hh"

/* ============================================================ AST ============================================================ */

Type::Type(yy::location loc, NodeType t, std::string n): name(n){
    AST::nodeType = t;
    AST::location = loc;
}
Type::Type(yy::location loc, NodeType t){
    AST::nodeType = t;
    AST::location = loc;
}
Type::Type(yy::location loc, NodeType t, std::shared_ptr<Type> tt): typeOfCompexType(tt){
    AST::nodeType = t;
    AST::location = loc;
}
Type::Type(yy::location loc, NodeType t, std::shared_ptr<NodeList> v): complexType(v){
    AST::nodeType = t;
    AST::location = loc;
}
Type::Type(yy::location loc, NodeType t, std::shared_ptr<NodeList> v, std::shared_ptr<Type> tt)
    : complexType(v), typeOfCompexType(tt){
        AST::nodeType = t;
        AST::location = loc;
}
std::string Type::print(){
    return "(TYPE"+std::to_string(nodeType)+")"; //TODO
}
Function::Function(yy::location loc, NodeType t, std::string n, std::shared_ptr<AST> a, std::shared_ptr<Type> r,
    std::shared_ptr<AST> b): name(n), argList(a), returnedType(r), body(b){
        AST::nodeType = t;
        AST::location = loc;
}
std::string Function::print(){
    return "(FUNCTION"+name+argList->print()+returnedType->print()+body->print()+")";
}
Return::Return(yy::location loc, NodeType t, std::shared_ptr<AST> e): expression(e){
    AST::location = loc;
}
std::string Return::print(){
    return "(RETURN"+std::to_string(nodeType)+expression->print()+")";
}
Statement::Statement(yy::location loc, NodeType t, std::shared_ptr<AST> c, std::shared_ptr<AST> b, std::shared_ptr<AST> e)
    : condition(c), body(b), elseBody(e){
        AST::nodeType = t;
        AST::location = loc;
}
Statement::Statement(yy::location loc, NodeType t, std::shared_ptr<AST> c, std::shared_ptr<AST> b)
    : condition(c), body(b){
        AST::nodeType = t;
        AST::location = loc;
}
std::string Statement::print(){
    return "(STATEMENT"+std::to_string(nodeType)+
    condition->print()+
    body->print()+(elseBody ? elseBody->print() : "")+")";
}
NodeList::NodeList(yy::location loc, NodeType t, std::shared_ptr<AST> e){
    AST::nodeType = t;
    AST::location = loc;
    list.push_back(e);
}
std::string NodeList::print(){
    if(list.size() == 1) return list[0]->print();
    std::string result = "(NODELIST"+std::to_string(nodeType)+"["+std::to_string(AST::location.begin.line)+"]";
    for(auto listElem : list) result += listElem->print();
    return result+")";
}
void NodeList::addNode(std::shared_ptr<AST> node){
    list.push_back(node);
}
unsigned NodeList::nrOfElements(){
    return list.size();
}
std::shared_ptr<AST> NodeList::giveMeOnlyElem(){
    return list[0];
}
Expression::Expression(yy::location loc, NodeType t, std::shared_ptr<AST> l, std::shared_ptr<AST> r)
    : left(l), right(r){
        AST::nodeType = t;
        AST::location = loc;
    }
Expression::Expression(yy::location loc, NodeType t, std::shared_ptr<AST> l): left(l){
    AST::nodeType = t;
    AST::location = loc;
}
std::string Expression::print(){
    if(right) return "(EXPRESSION"+std::to_string(nodeType)+left->print()+right->print()+")";
    else return "(EXPRESSION"+std::to_string(nodeType)+left->print()+")";
}
Special::Special(yy::location loc, NodeType t){
    AST::nodeType = t;
    AST::location = loc;
}
std::string Special::print(){
    return "(SPECIAL"+std::to_string(nodeType)+")";
}
// template<typename T> Literal::Literal(T val) : value(val){}
Literal::Literal(yy::location loc, char val): value(val){
    AST::nodeType = NodeType::CHAR_LITERAL;
    AST::location = loc;
}
Literal::Literal(yy::location loc, long long val): value(val){
    AST::nodeType = NodeType::I64_LITERAL;
    AST::location = loc;
}
Literal::Literal(yy::location loc, bool val): value(val){
    AST::nodeType = NodeType::BOOL_LITERAL;
    AST::location = loc;
}
Literal::Literal(yy::location loc, double val): value(val){
    AST::nodeType = NodeType::F64_LITERAL;
    AST::location = loc;
}
Literal::Literal(yy::location loc, std::string val): value(val){
    AST::nodeType = NodeType::STRING_LITERAL;
    AST::location = loc;
}
std::string Literal::print(){
    // const auto a = std::get<1>(value);
    return "(LITERAL["+std::to_string(AST::location.begin.line)+"]"+std::to_string(value.index())+")";
}
ID::ID(yy::location loc, std::string n): name(n){
    AST::nodeType = NodeType::IDENT;
    AST::location = loc;
}
std::string ID::print(){
    return "(ID["+std::to_string(AST::location.begin.line)+"]"+name+")";
}
Object::Object(yy::location loc, std::shared_ptr<AST> v): value(v){
    AST::nodeType = NodeType::OBJECT_LITERAL;
    AST::location = loc;
}
std::string Object::print(){
    return "(OBJECT["+
        std::to_string(AST::location.begin.line)+
        "]"+value->print()+")";
}
List::List(yy::location loc, std::shared_ptr<AST> v): value(v){
    AST::nodeType = NodeType::LIST_LITERAL;
    AST::location = loc;
}
std::string List::print(){
    return "(LIST["+
        std::to_string(AST::location.begin.line)+
        "]"+value->print()+")";
}
Argument::Argument(yy::location loc, std::string id, std::shared_ptr<Type> tof): ident(id), typeOfArg(tof){
    AST::nodeType = NodeType::ARGUMENT;
    AST::location = loc;
}
std::string Argument::print(){
    return "(ARG)"; //TODO
}
Struct::Struct(yy::location loc, NodeType t, std::string id, std::shared_ptr<NodeList> l)
    : ident(id), listOfFields(l){
        AST::nodeType = t;
        AST::location = loc;
}
std::string Struct::print(){
    return "(STRUCT"+ident+listOfFields->print()+")";
}

/* ======================================================= Semantic ======================================================= */

bool Type::operator ==(Type const &snd){
    if(this->nodeType != snd.nodeType) return false;
    else if(this->nodeType == NodeType::IDENT && this->name != snd.name) return false;
    else if(this->nodeType == NodeType::ARRAY && this->typeOfCompexType != snd.typeOfCompexType) return false;
    else if(this->nodeType == NodeType::OBJECT && this->complexType != snd.complexType) return false;
    else if(this->nodeType == NodeType::FUNCTION &&
        this->complexType != snd.complexType &&
        this->typeOfCompexType != snd.typeOfCompexType) return false;
    return true;
}
std::shared_ptr<Function> NodeList::findMainFunction(){
    unsigned found = 0;
    std::shared_ptr<Function> result;
    for(const auto& a : list){
        if(a->nodeType == NodeType::FUNCTION && a->getName() == "main"){
            found++;
            result = std::static_pointer_cast<Function>(a);
        }
    }
    if(found == 0){
        std::cerr<<"Error. No function main\n";
        exit(EXIT_FAILURE);
    } else if(found > 1){
        std::cerr<<"Error. More than one function main\n";
        exit(EXIT_FAILURE);
    } else if(result->nrOfArguments() > 1){
        std::cerr<<"Error. Function main has more than 1 argument\n";
        exit(EXIT_FAILURE);
    }
    return result;
}
std::map<std::string, std::shared_ptr<AST>> NodeList::getIDs(){
    std::map<std::string, std::shared_ptr<AST>> result;
    for(const auto& a : list){
        std::map<std::string, std::shared_ptr<AST>>::iterator it = result.find(a->getName());
        if(it != result.end()){
            std::cerr<<"Error in file: "<<a->location<<". Reusing identifier "<<a->getName()<<"\n";
            exit(EXIT_FAILURE);
        }
        result[a->getName()] = a;
    }
    return result;
}
std::string AST::getName(){
    return "";
}
/*TODO:
    - check all types, all nodes (and nodeList) have to have types
    -- for nodeList it's type of last node
    -- for literals it's obvius
    -- matching functions to their calls
*/
void Function::setInnerType(SemanticAnalyzerHelper sah){
    std::cout<<"SETTING STATIC TYPE FOR FUNCTION";
    if(innerType == nullptr){
        //TODO 
    }
    body->setInnerType(sah);
}
std::string Function::getName(){
    return name;
}
unsigned Function::nrOfArguments(){
    if(argList->nodeType == NodeType::EMPTY) return 0;
     else if(argList->nodeType == NodeType::ARGLIST){
        return std::static_pointer_cast<NodeList>(argList)->nrOfElements();
    } else {
        std::cerr<<"Internal error. Wrong type of argument list\n";
        exit(EXIT_FAILURE);
    }
}
std::string Struct::getName(){
    return ident;
}
SemanticAnalyzerHelper::SemanticAnalyzerHelper(std::map<std::string, std::shared_ptr<AST>> globals): globalIds(globals){}