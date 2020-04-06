#pragma once
#include <string>
#include <variant>
#include <memory>
#include <vector>
#include <map>
#include "../location.hh"

enum NodeType{
    SWITCH, CASE, EVERY, WHILE, DO_WHILE, IF, ASS, NDT_AND, NDT_OR,
    LIMIT, NDT_NOT, LOG_OR, LOG_AND, BIT_OR, BIT_XOR, 
    BIT_AND, EQU, NEQU, LESS, GREATER, LEQ, GEQ, SHL, SHR, ADD, SUB, MUL,
    DIV, MOD, MINUS, LOG_NOT, BIT_NOT, DOT, ARRAY_ELEM, CALL,
    CONTINUE, BREAK, NIL, SUCCESS, FAIL, NOP, UNDERSCORE, EMPTY,
    STRUCTLIST, DATALIST, SEPARATOR, CASE_LIST, COMMA,
    TYPELIST, ARGLIST, RET, YIELD, IDENT, OBJECT, FUNCTION,
    I64, F64, STRING, CHAR, BOOL, VOID, ARRAY, STR, DATA,
    LAMBDA, OPERATOR_OVER, OBJECT_LITERAL, LIST_LITERAL, ARGUMENT,
    CHAR_LITERAL, I64_LITERAL, BOOL_LITERAL, F64_LITERAL, STRING_LITERAL
};
struct SemanticAnalyzerHelper;
struct AST{
    virtual std::string print() = 0;
    yy::location location;
    NodeType nodeType;
    virtual std::string getName();
    virtual void setStaticType(SemanticAnalyzerHelper sah);
};
struct Function;
struct Type;
struct NodeList : AST{
    NodeList(yy::location loc, NodeType t, std::shared_ptr<AST> e);
    std::string print();
    void addNode(std::shared_ptr<AST> node);
    unsigned nrOfElements();
    std::shared_ptr<AST> giveMeOnlyElem();
    // Semantic
    std::map<std::string, std::shared_ptr<AST>> getIDs();
    std::shared_ptr<Function> findMainFunction();
private:
    std::vector<std::shared_ptr<AST>> list;
};
struct Type : AST{
    Type(yy::location loc, NodeType t);
    Type(yy::location loc, NodeType t, std::string n);
    Type(yy::location loc, NodeType t, std::shared_ptr<Type> tt);
    Type(yy::location loc, NodeType t, std::shared_ptr<NodeList> v);
    Type(yy::location loc, NodeType t, std::shared_ptr<NodeList> v, std::shared_ptr<Type> tt);
    std::string print();
private:
    std::string name;
    std::shared_ptr<Type> typeOfCompexType;
    std::shared_ptr<NodeList> complexType;
    bool operator ==(Type const &snd);
    // bool operator !=(Type const &snd){
    //     return !(this == snd);
    // }
};
struct Function : AST{
    Function(yy::location loc, NodeType t, std::string n, std::shared_ptr<AST> a, 
        std::shared_ptr<Type> r, std::shared_ptr<AST> b);
    std::string print();
    std::string getName() override;
    unsigned nrOfArguments();
    void setStaticType(SemanticAnalyzerHelper sah) override;
private:
    std::string name;
    std::shared_ptr<AST> argList;
    std::shared_ptr<Type> returnedType;
    std::shared_ptr<AST> body;
};
struct Return : AST{
    Return(yy::location loc, NodeType t, std::shared_ptr<AST> e);
    std::string print();
private:
    std::shared_ptr<AST> expression;
};
struct Statement : AST{
    Statement(yy::location loc, NodeType t, std::shared_ptr<AST> c, std::shared_ptr<AST> b, std::shared_ptr<AST> e);
    Statement(yy::location loc, NodeType t, std::shared_ptr<AST> c, std::shared_ptr<AST> b);
    std::string print();
private:
    std::shared_ptr<AST> condition;
    std::shared_ptr<AST> body;
    std::shared_ptr<AST> elseBody;
};
struct Expression : AST{
    Expression(yy::location loc, NodeType t, std::shared_ptr<AST> l, std::shared_ptr<AST> r);
    Expression(yy::location loc, NodeType t, std::shared_ptr<AST> l);
    std::string print();
private:
    std::shared_ptr<AST> left;
    std::shared_ptr<AST> right;
};
struct Special : AST{
    Special(yy::location loc, NodeType t);
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
struct Argument : AST{
    Argument(yy::location loc, std::string id, std::shared_ptr<Type> type);
    std::string print();
private:
    std::string ident;
    std::shared_ptr<Type> typeOfArg;
};
struct Struct : AST{
    Struct(yy::location loc, NodeType t, std::string id, std::shared_ptr<NodeList> l);
    std::string print();
    std::string getName() override;
private:
    std::string ident;
    std::shared_ptr<NodeList> listOfFields;
};

/* ======================================================= Semantic ======================================================= */

struct SemanticAnalyzerHelper{
    SemanticAnalyzerHelper(std::map<std::string, std::shared_ptr<AST>> globals);
    std::map<std::string, std::shared_ptr<AST>> globalIds;
    std::map<std::string, std::shared_ptr<Type>> localIds;
};