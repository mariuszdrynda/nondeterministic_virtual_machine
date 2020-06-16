#include "ast.hpp"
#include <string>
#include "../location.hh"

unsigned loopNr = 0;
unsigned ifNr = 0;
unsigned statementNr = 0;

std::string str(NodeType nt){
    switch(nt){
        case NodeType::SEPARATOR: return "SEMICOLON";
        case NodeType::FUNCTION: return "FUN";
        case NodeType::YIELD: return "YIELD";
        case NodeType::RET: return "RET";
        case NodeType::ASS: return "ASSIGN";
        case NodeType::EVERY: return "EVERY";
        case NodeType::NDT_AND: return "NDT_AND";
        case NodeType::WHILE: return "WHILE";
        case NodeType::DO_WHILE: return "DOWHILE";
        case NodeType::IF: return "IF";
        case NodeType::BREAK: return "BREAK";
        case NodeType::CONTINUE: return "CONTINUE";
        case NodeType::COMMA: return "COMMA";
        case NodeType::NDT_NOT: return "NDT_NOT";
        case NodeType::LOG_OR: return "LOG_OR";
        case NodeType::LOG_AND: return "LOG_AND";
        case NodeType::EQU: return "EQU";
        case NodeType::NEQU: return "NEQU";
        case NodeType::GEQ: return "GEQ";
        case NodeType::LEQ: return "LEQ";
        case NodeType::LESS: return "LESS";
        case NodeType::GREATER: return "GREATER";
        case NodeType::ADD: return "ADD";
        case NodeType::SUB: return "SUB";
        case NodeType::MUL: return "MUL";
        case NodeType::DIV: return "DIV";
        case NodeType::MOD: return "MOD";
        case NodeType::MINUS: return "MINUS";
        case NodeType::LOG_NOT: return "LOG_NOT";
        case NodeType::CALL: return "CALL";
        case NodeType::DOT: return "DOT";
        case NodeType::OBJECT: return "OBJECT";
        case NodeType::ARRAY_ELEM: return "ARRAY_ELEM";
        case NodeType::IDENT: return "IDE";
        case NodeType::NIL: return "NIL";
        case NodeType::BOOL_LITERAL: return "BOOL";
        case NodeType::CHAR_LITERAL: return "CHAR";
        case NodeType::I64_LITERAL: return "I64";
        case NodeType::F64_LITERAL: return "F64";
        case NodeType::STRING_LITERAL: return "STRING";
        case NodeType::MAKE_LIST: return "MAKE_LIST";
        case NodeType::EMPTY: return "EMPTY";
        case NodeType::PROGRAMLIST: return "PROGRAMLIST";
    }
    return "";
}

/* ============================================================ AST ============================================================ */

Function::Function(yy::location loc, NodeType t, std::string n, std::shared_ptr<AST> a, std::shared_ptr<AST> b)
    : name(n), argList(a), body(b){
        AST::nodeType = t;
        AST::location = loc;
}
std::string Function::print(){
    return "(FUNCTION"+name+argList->print()+body->print()+")";
}
void Function::semantic(SemanticAnalyzerHelper sc){
    body->semantic(sc);
}
std::string Function::generatr_ir(){
    return "FUN "+name+" "+argList->generatr_ir()+"\n"+body->generatr_ir()+"ENDFUN\n";
}

Return::Return(yy::location loc, NodeType t, std::shared_ptr<AST> e): expression(e){
    AST::nodeType = t;
    AST::location = loc;
}
std::string Return::print(){
    return "(RETURN"+str(nodeType)+expression->print()+")";
}
void Return::semantic(SemanticAnalyzerHelper sc){
    if(sc.assign == 1) throw str(nodeType)+" on the left side of assignment expression";
    else expression->semantic(sc);
}
std::string Return::generatr_ir(){
    return expression->generatr_ir()+str(nodeType)+"\n";
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
void Statement::semantic(SemanticAnalyzerHelper sc){
    if(sc.assign == 1) throw str(nodeType)+" on the left side of assignment expression";
    else{
        condition->semantic(SemanticAnalyzerHelper(0));
        body->semantic(SemanticAnalyzerHelper(0));
        if(elseBody != nullptr && elseBody->nodeType != NodeType::EMPTY){
            elseBody->semantic(SemanticAnalyzerHelper(0));
        }
    }
}
std::string Statement::generatr_ir(){
        unsigned curr = loopNr;
        ++loopNr;
        if(nodeType == NodeType::WHILE)
            return "WHILE "+std::to_string(curr)+"\n"+condition->generatr_ir()+"DO "+std::to_string(curr)+"\n"+body->generatr_ir()+"ENDWHILE "+std::to_string(curr)+"\n";
        else if(nodeType == NodeType::DO_WHILE)
            return "DOWHILE "+std::to_string(curr)+"\n"+body->generatr_ir()+"DOWHILEBODY "+std::to_string(curr)+"\n"+condition->generatr_ir()+"ENDDOWHILE "+std::to_string(curr)+"\n";
        else if(nodeType == NodeType::EVERY)
            return "EVERY "+std::to_string(curr)+"\n"+condition->generatr_ir()+"EVERYBODY "+std::to_string(curr)+"\n"+body->generatr_ir()+"ENDEVERY "+std::to_string(curr)+"\n";
        else if(nodeType == NodeType::IF){
            curr = ifNr;
            ++ifNr;
            std::string builder = "IF "+std::to_string(curr)+"\n"+condition->generatr_ir()+"THEN "+std::to_string(curr)+"\n"+body->generatr_ir()+"ELSE "+std::to_string(curr)+"\n";
            if(elseBody->nodeType != NodeType::EMPTY){
                builder += elseBody->generatr_ir();
            }
            builder += "ENDIF "+std::to_string(curr)+"\n";
            return builder;
        }
        else
            throw "Internal error. Wrong statement type.";
}

std::string Statement::print(){
    return "(STATEMENT"+str(nodeType)+
    condition->print()+
    body->print()+(elseBody ? elseBody->print() : "")+")";
}
NodeList::NodeList(yy::location loc, NodeType t){
    AST::nodeType = t;
    AST::location = loc;
}
NodeList::NodeList(yy::location loc, NodeType t, std::shared_ptr<AST> e){
    AST::nodeType = t;
    AST::location = loc;
    list.push_back(e);
}
std::string NodeList::print(){
    if(list.size() == 1) return list[0]->print();
    std::string result = "(NODELIST"+str(nodeType)+"["+std::to_string(AST::location.begin.line)+"]";
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
void NodeList::semantic(SemanticAnalyzerHelper sc){
    for(auto& a : list){
        a->semantic(sc);
    }
}
std::string NodeList::generatr_ir(){
    if(nodeType == NodeType::SEPARATOR){
        std::string ir = "";
        for(int i = 0; i < list.size()-1; i++) ir += list[i]->generatr_ir()+"SEMICOLON\n";
        // for(auto& a : list)
        return ir+list.back()->generatr_ir();
    } else if(nodeType == NodeType::COMMA || nodeType == NodeType::NDT_OR){
        std::string ir = "";
        for(auto& a : list) ir += a->generatr_ir();
        return ir+str(nodeType)+" "+std::to_string(list.size())+"\n";
    } else if(nodeType == NodeType::PROGRAMLIST){
        std::string ir = "";
        for(auto& a : list) ir += a->generatr_ir();
        return ir;
    } else if(nodeType == NodeType::STRUCTLIST){
        std::string ir = ""; //todo
        for(auto& a : list) ir += a->getName() + " ";
        return std::to_string(list.size())+" "+ir;
    } else if(nodeType == NodeType::ARGLIST){
        std::string ir = "";
        for(auto& a : list) ir += a->getName() + " ";
        return std::to_string(list.size())+" "+ir;
    }
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
    if(right) return "(EXPRESSION"+str(nodeType)+left->print()+right->print()+")";
    else return "(EXPRESSION"+str(nodeType)+left->print()+")";
}
void Expression::semantic(SemanticAnalyzerHelper sc){
    side = sc.assign;
    if(sc.assign == 1){
        if(!(nodeType == NodeType::DOT || nodeType == NodeType::ARRAY_ELEM))
            throw str(nodeType)+" on the left side of assignment expression";
        else{
            left->semantic(SemanticAnalyzerHelper(0));
            if(right != nullptr) right->semantic(SemanticAnalyzerHelper(0));
        }
    } else if(nodeType == NodeType::ASS){
        left->semantic(SemanticAnalyzerHelper(1));
        if(right != nullptr) right->semantic(SemanticAnalyzerHelper(-1));
    } else{
        left->semantic(sc);
        if(right != nullptr) right->semantic(sc);
    }
}
std::string Expression::generatr_ir(){
    if(nodeType == NodeType::ASS)
        return left->generatr_ir()+right->generatr_ir()+"ASSIGN\n";
    else if(nodeType == NodeType::MINUS)
        return left->generatr_ir()+"MINUS\n";
    else if(nodeType == NodeType::BIT_NOT || nodeType == NodeType::LOG_NOT || nodeType == NodeType::NDT_NOT || nodeType == NodeType::MAKE_LIST)
        return left->generatr_ir()+str(nodeType)+"\n";
    else if(nodeType == NodeType::NDT_AND)
        return left->generatr_ir()+right->generatr_ir()+str(nodeType)+" "+std::to_string(loopNr-1)+"\n";
    else if(nodeType == NodeType::ARRAY_ELEM && side == 1)
        return left->generatr_ir()+right->generatr_ir()+"REF_ARR_ELEM\n";
    else if(nodeType == NodeType::DOT && side == 1)
        return left->generatr_ir()+right->generatr_ir()+"REF_FIELD\n";
    else if(nodeType == NodeType::CALL && side != 1)
        return left->generatr_ir()+right->generatr_ir()+"ARGUMENT\nCALL\n";
    else{
        return left->generatr_ir()+right->generatr_ir()+str(nodeType)+"\n";
    }
}

Special::Special(yy::location loc, NodeType t){
    AST::nodeType = t;
    AST::location = loc;
}
std::string Special::print(){
    return "(SPECIAL"+str(nodeType)+")";
}
void Special::semantic(SemanticAnalyzerHelper sc){
    if((nodeType == NodeType::BREAK || nodeType == NodeType::CONTINUE) && (sc.assign == 1 || sc.assign == -1))
        throw str(nodeType)+" inside assignment expression";
}
std::string Special::generatr_ir(){
    if(nodeType == NodeType::EMPTY) return "EMPTY\n";
    return str(nodeType)+" "+std::to_string(loopNr-1)+"\n";
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
void Literal::semantic(SemanticAnalyzerHelper sc){
    if(sc.assign == 1) throw "Literal on the left side of assignment expression";
}
std::string Literal::generatr_ir(){
    std::string val;
    switch(value.index()){
        case 0: val = std::to_string(std::get<0>(value)); break;
        case 1: val += std::get<1>(value); break;
        case 2: val = std::get<2>(value)? "TRUE" : "FALSE"; break;
        case 3: val = std::to_string(std::get<3>(value)); break;
        case 4: val = std::get<4>(value); break;
    }
    return str(nodeType)+" "+val+"\n";
}

ID::ID(yy::location loc, std::string n): name(n){
    AST::nodeType = NodeType::IDENT;
    AST::location = loc;
}
std::string ID::print(){
    return "(ID["+std::to_string(AST::location.begin.line)+"]"+name+")";
}
std::string ID::getName(){
    return name;
}
void ID::semantic(SemanticAnalyzerHelper sc){
    side = sc.assign;
    return; //OK
}
std::string ID::generatr_ir(){
    return "IDE "+name+"\n";
}

Struct::Struct(yy::location loc, NodeType t, std::string id, std::shared_ptr<NodeList> l)
    : ident(id), listOfFields(l){
        AST::nodeType = t;
        AST::location = loc;
}
std::string Struct::print(){
    return "(STRUCT"+ident+listOfFields->print()+")";
}
void Struct::semantic(SemanticAnalyzerHelper sc){
    return; //ok
}
std::string Struct::generatr_ir(){
    return "STRUCT "+this->ident+" "+this->listOfFields->generatr_ir()+"\n";
}

/* ======================================================= Semantic ======================================================= */

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
SemanticAnalyzerHelper::SemanticAnalyzerHelper(short ass){
    assign = ass;
}