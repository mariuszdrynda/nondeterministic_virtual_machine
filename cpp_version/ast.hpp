#pragma once
#include <string>
#include <variant>
#include <memory>
#include <vector>
#include "location.hh"

// enum NodeType{
//     SWITCH, CASE, EVERY, WHILE, DO_WHILE, IF, ASS, NDT_AND, NDT_OR,
//     LIMIT, NDT_NOT, LOG_OR, LOG_AND, BIT_OR, BIT_XOR, 
//     BIT_AND, EQU, NEQU, LESS, GREATER, LEQ, GEQ, SHL, SHR, ADD, SUB, MUL,
//     DIV, MOD, MINUS, LOG_NOT, BIT_NOT, DOT, ARRAY_ELEM, CALL,
//     CONTINUE, BREAK, NIL, SUCCESS, FAIL, NOP, UNDERSCORE, EMPTY,
//     STRUCTLIST, DATALIST, SEPARATOR, CASE_LIST, COMMA,
//     TYPELIST, ARGLIST, RET, YIELD, IDENT, OBJECT, FUNCTION,
//     I64, F64, STRING, CHAR, BOOL, VOID, ARRAY, STR, DATA
// };
enum StatementType{
    SWITCH, CASE, EVERY, WHILE, DO_WHILE, IF
};
enum ExpressionType{
    ASS, NDT_AND, NDT_OR, LIMIT, NDT_NOT, LOG_OR, LOG_AND, BIT_OR, BIT_XOR, 
    BIT_AND, EQU, NEQU, LESS, GREATER, LEQ, GEQ, SHL, SHR, ADD, SUB, MUL,
    DIV, MOD, MINUS, LOG_NOT, BIT_NOT, DOT, ARRAY_ELEM, CALL
};
enum SpecialType{
    CONTINUE, BREAK, NIL, SUCCESS, FAIL, NOP, UNDERSCORE, EMPTY
};
enum NodeListType{
    STRUCTLIST, DATALIST, SEPARATOR, CASE_LIST, COMMA, TYPELIST, ARGLIST
};
enum ReturnType{
    RET, YIELD
};
enum TypeType{
    IDENT, OBJECT, FUNCTION, I64, F64, STRING, CHAR, BOOL, VOID, ARRAY
};
enum StructType{
    STR, DATA
};
struct AST{
    virtual std::string print() = 0;
    yy::location location;
    // NodeType nodeType;
};
struct Function;
struct NodeList : AST{
    NodeList(yy::location loc, NodeListType t, std::shared_ptr<AST> e);
    std::string print();
    void addNode(std::shared_ptr<AST> node);
    bool hasOneElement();
    std::shared_ptr<AST> giveMeOnlyElem();
    // Semantic
    std::shared_ptr<Function> findMainFunction();
private:
    std::vector<std::shared_ptr<AST>> list;
    NodeListType type;
};
struct Type : AST{
    Type(yy::location loc, TypeType t);
    Type(yy::location loc, TypeType t, std::string n);
    Type(yy::location loc, TypeType t, std::shared_ptr<Type> tt);
    Type(yy::location loc, TypeType t, std::shared_ptr<NodeList> v);
    Type(yy::location loc, TypeType t, std::shared_ptr<NodeList> v, std::shared_ptr<Type> tt);
    std::string print();
private:
    TypeType type;
    std::string name;
    std::shared_ptr<Type> typeOfCompexType;
    std::shared_ptr<NodeList> complexType;
};
struct Function : AST{
    Function(yy::location loc, std::string n, std::shared_ptr<AST> a, 
        std::shared_ptr<Type> r, std::shared_ptr<AST> b);
    std::string print();
private:
    std::string name;
    std::shared_ptr<AST> argList;
    std::shared_ptr<Type> returnedType;
    std::shared_ptr<AST> body;
};
struct Return : AST{
    Return(yy::location loc, ReturnType t, std::shared_ptr<AST> e);
    std::string print();
private:
    ReturnType returnType;
    std::shared_ptr<AST> expression;
};
struct Statement : AST{
    Statement(yy::location loc, StatementType type, std::shared_ptr<AST> c, std::shared_ptr<AST> t, std::shared_ptr<AST> e);
    Statement(yy::location loc, StatementType type, std::shared_ptr<AST> c, std::shared_ptr<AST> t);
    std::string print();
private:
    StatementType statementType;
    std::shared_ptr<AST> condition;
    std::shared_ptr<AST> body;
    std::shared_ptr<AST> elseBody;
};
struct Expression : AST{
    Expression(yy::location loc, ExpressionType t, std::shared_ptr<AST> l, std::shared_ptr<AST> r);
    Expression(yy::location loc, ExpressionType t, std::shared_ptr<AST> l);
    std::string print();
private:
    ExpressionType expressionType;
    std::shared_ptr<AST> left;
    std::shared_ptr<AST> right;
};
struct Special : AST{
    Special(yy::location loc, SpecialType t);
    std::string print();
private:
    SpecialType type;
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
    Struct(yy::location loc, StructType s, std::string id, std::shared_ptr<NodeList> l);
    std::string print();
private:
    StructType structType;
    std::string ident;
    std::shared_ptr<NodeList> listOfFields;
};