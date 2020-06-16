#pragma once
#include <string>
#include <variant>
#include <memory>
#include <vector>
#include <map>
#include "../location.hh"

enum class NodeType{
    EVERY, WHILE, DO_WHILE, IF, ASS, NDT_AND, NDT_OR, LIMIT, NDT_NOT, 
    LOG_OR, LOG_AND, BIT_OR, BIT_XOR, BIT_AND, EQU, NEQU, LESS, GREATER, LEQ,
    GEQ, SHL, SHR, ADD, SUB, MUL, DIV, MOD, MINUS, LOG_NOT,
    BIT_NOT, DOT, ARRAY_ELEM, CALL, CONTINUE, BREAK, NIL, SUCCESS, FAIL,
    EMPTY, STRUCTLIST, DATALIST, SEPARATOR, COMMA, ARGLIST, RET,
    YIELD, IDENT, OBJECT, FUNCTION, I64, F64, STRING, CHAR, BOOL, VOID,
    ARRAY, STR, OBJECT_LITERAL, LIST_LITERAL, CHAR_LITERAL, I64_LITERAL,
    BOOL_LITERAL, F64_LITERAL, STRING_LITERAL, PROGRAMLIST, MAKE_LIST
};
struct SemanticAnalyzerHelper;
struct AST{
    virtual std::string print() = 0;
    virtual void semantic(SemanticAnalyzerHelper sc) = 0;
    virtual std::string generatr_ir() = 0;
    virtual std::string getName();
    yy::location location;
    NodeType nodeType;
};
struct Function;
struct NodeList : AST{
    NodeList(yy::location loc, NodeType t);
    NodeList(yy::location loc, NodeType t, std::shared_ptr<AST> e);
    std::string print();
    void semantic(SemanticAnalyzerHelper sc);
    std::string generatr_ir();
    void addNode(std::shared_ptr<AST> node);
    unsigned nrOfElements();
    std::shared_ptr<AST> giveMeOnlyElem();
    // Semantic
    std::map<std::string, std::shared_ptr<AST>> getIDs();
    std::shared_ptr<Function> findMainFunction();
private:
    std::vector<std::shared_ptr<AST>> list;
};
struct Function : AST{
    Function(yy::location loc, NodeType t, std::string n, std::shared_ptr<AST> a, std::shared_ptr<AST> b);
    std::string print();
    void semantic(SemanticAnalyzerHelper sc);
    std::string generatr_ir();
    std::string getName() override;
    unsigned nrOfArguments();
private:
    std::string name;
    std::shared_ptr<AST> argList;
    std::shared_ptr<AST> body;
};
struct Return : AST{
    Return(yy::location loc, NodeType t, std::shared_ptr<AST> e);
    std::string print();
    void semantic(SemanticAnalyzerHelper sc);
    std::string generatr_ir();
private:
    std::shared_ptr<AST> expression;
};
struct Statement : AST{
    Statement(yy::location loc, NodeType t, std::shared_ptr<AST> c, std::shared_ptr<AST> b, std::shared_ptr<AST> e);
    Statement(yy::location loc, NodeType t, std::shared_ptr<AST> c, std::shared_ptr<AST> b);
    std::string print();
    void semantic(SemanticAnalyzerHelper sc);
    std::string generatr_ir();
private:
    std::shared_ptr<AST> condition;
    std::shared_ptr<AST> body;
    std::shared_ptr<AST> elseBody = nullptr;
};
struct Expression : AST{
    Expression(yy::location loc, NodeType t, std::shared_ptr<AST> l, std::shared_ptr<AST> r);
    Expression(yy::location loc, NodeType t, std::shared_ptr<AST> l);
    std::string print();
    void semantic(SemanticAnalyzerHelper sc);
    std::string generatr_ir();
private:
    std::shared_ptr<AST> left;
    std::shared_ptr<AST> right = nullptr;
    short side = -2;
};
struct Special : AST{
    Special(yy::location loc, NodeType t);
    std::string print();
    void semantic(SemanticAnalyzerHelper sc);
    std::string generatr_ir();
};
struct Literal : AST{
    //template<typename T> Literal(T val);
    Literal(yy::location loc, char val);
    Literal(yy::location loc, long long val);
    Literal(yy::location loc, bool val);
    Literal(yy::location loc, double val);
    Literal(yy::location loc, std::string val);
    std::string print();
    void semantic(SemanticAnalyzerHelper sc);
    std::string generatr_ir();
private:
    std::variant<long long, char, bool, double, std::string> value;
};
struct ID : AST{
    ID(yy::location loc, std::string n);
    std::string print();
    void semantic(SemanticAnalyzerHelper sc);
    std::string generatr_ir();
    std::string getName();
    std::string name;
private:
    short side = -2;
};
struct Struct : AST{
    Struct(yy::location loc, NodeType t, std::string id, std::shared_ptr<NodeList> l);
    std::string print();
    void semantic(SemanticAnalyzerHelper sc);
    std::string generatr_ir();
    std::string getName();
private:
    std::string ident;
    std::shared_ptr<NodeList> listOfFields;
};

/* ======================================================= Semantic ======================================================= */

struct SemanticAnalyzerHelper{
    SemanticAnalyzerHelper(short ass);
    short assign; //1 - left, -1 - right, 0 - none
};