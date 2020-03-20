#pragma once
#include <vector>
#include <string>
using namespace std;
// enum ASTType{
//     FN, NOT_LAZY, LAZY, RET, YIELD, ASS, SWITCH, CASE_LIST, CASE,
//     EVERY, WHILE, DO_WHILE, IF, CONTINUE, BREAK, COMMA,
//     NDTAND, NDTOR, LIMIT, LOGNOT, LOGOR, LOGAND, 
//     BIT_OR, BIT_XOR, BIT_AND, EQU, NEQ, GRE, LEQ, GEQ,
//     SHL, SHR, ADD, MIN, MUL, DIV, MOD, MINUS, NEG, BITNEG,
//     DOT, ARRAY_ELEMENT, CALL, ID, NIL, SELF, FALSE, TRUE,
//     I64_LITERAL, CHAR_LITERAL, F64_LITERAL, STRING_LITERAL,
//     SUCCESS, FAIL, NOP, LIST, LAMBDA, ARGLIST, EMPTY,
//     TYPE_ID, TYPE_I64, TYPE_F64,
//     TYPE_STRING, TYPE_CHAR, TYPE_BOOL, TYPE_VOID, TYPE_ARRAY
// };
struct Node;
struct AST{
    const Node* type = nullptr;
};
struct NodeList : public AST{
    NodeList(unsigned line, string type, Node* node);
    void addNode(Node* node);
    string printList();
    bool hasOneElement();
    Node* giveMeOnlyElem();
private:
    unsigned line = 0;
    string listType = "";
    vector<Node*> list;
};
struct Node : public AST{
    string print();
    Node(unsigned line, string m_name);
    Node(unsigned line, NodeList* childList);
    Node(unsigned line, string m_name, string value);
    Node(unsigned line, string m_name, long long value);
    Node(unsigned line, string m_name, double value);
    Node(unsigned line, string m_name, char value);
    Node(unsigned line, string m_name, Node* child1);
    Node(unsigned line, string m_name, NodeList* childList);
    Node(unsigned line, string m_name, string value, Node* child1);
    Node(unsigned line, string m_name, string value, NodeList* childList);
    Node(unsigned line, string m_name, Node* child1, Node* child2);
    Node(unsigned line, string m_name, Node* child1, NodeList* childList);
    Node(unsigned line, string m_name, Node* child1, Node* child2, NodeList* childList);
    Node(unsigned line, string m_name, Node* child1, NodeList* childList1, NodeList* childList2);
    Node(unsigned line, string m_name, Node* child1, Node* child2, NodeList* childList, Node* child3, NodeList* childList2);
    Node(unsigned line, string m_name, Node* child1, string strval, NodeList* childList1, Node* child2, NodeList* childList2);
private:
    unsigned line = 0;
    string name = "";
    string strval = "";
    long long i64val = 0;
    double f64val = 0.0;
    char charval = '\0';
    Node* children1 = nullptr;
    Node* children2 = nullptr;
    Node* children3 = nullptr;
    NodeList* childrenList1 = nullptr;
    NodeList* childrenList2 = nullptr;
};