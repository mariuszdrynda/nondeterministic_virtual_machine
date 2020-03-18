#include "node.hpp"
string Node::print(){
    return "("+(this->name)+"["+to_string(this->line)+"]"+
        (this->strval != "" ? (this->strval) :"")+
        (this->i64val != 0 ? to_string(this->i64val) : "")+
        (this->f64val != 0 ? to_string(this->f64val) : "")+
        (this->charval != 0 ? to_string(this->charval) : "")+
        (this->children1 != nullptr ? children1->print() :"")+
        (this->children2 != nullptr ? children2->print() :"")+
        (this->children3 != nullptr ? children3->print() :"")+
        (this->childrenList1 != nullptr ? childrenList1->printList() :"")+
        (this->childrenList2 != nullptr ? childrenList2->printList() :"")+")";
}
Node::Node(unsigned line, string m_name){
    this->line = line;
    this->name = m_name;
}
Node::Node(unsigned line, NodeList* childList){
    this->line = line;
    this->childrenList1 = childList;
}
Node::Node(unsigned line, string m_name, string value){
    this->line = line;
    this->name = m_name;
    this->strval = value;
}
Node::Node(unsigned line, string m_name, long long value){
    this->line = line;
    this->name = m_name;
    this->i64val = value;
}
Node::Node(unsigned line, string m_name, double value){
    this->line = line;
    this->name = m_name;
    this->f64val = value;
}
Node::Node(unsigned line, string m_name, char value){
    this->line = line;
    this->name = m_name;
    this->charval = value;
}
Node::Node(unsigned line, string m_name, Node* child1){
    this->line = line;
    this->name = m_name;
    this->children1 = child1;
}
Node::Node(unsigned line, string m_name, string value, Node* child1){
    this->line = line;
    this->name = m_name;
    this->strval = value;
    this->children1 = child1;
}
Node::Node(unsigned line, string m_name, string value, NodeList* childList){
    this->line = line;
    this->name = m_name;
    this->strval = value;
    this->childrenList1 = childList;
}
Node::Node(unsigned line, string m_name, Node* child1, Node* child2){
    this->line = line;
    this->name = m_name;
    this->children1 = child1;
    this->children2 = child2;
}
Node::Node(unsigned line, string m_name, Node* child1, NodeList* childList){
    this->line = line;
    this->name = m_name;
    this->children1 = child1;
    this->childrenList1 = childList;
}
Node::Node(unsigned line, string m_name, Node* child1, Node* child2, NodeList* childList){
    this->line = line;
    this->name = m_name;
    this->children1 = child1;
    this->children2 = child2;
    this->childrenList1 = childList;
}
Node::Node(unsigned line, string m_name, Node* child1, NodeList* childList1, NodeList* childList2){
    this->line = line;
    this->name = m_name;
    this->children1 = child1;
    this->childrenList1 = childList1;
    this->childrenList2 = childList2;
}

Node::Node(unsigned line, string m_name, Node* child1, Node* child2, NodeList* childList1, Node* child3, NodeList* childList2){
    this->line = line;
    this->name = m_name;
    this->children1 = child1;
    this->children2 = child2;
    this->childrenList1 = childList1;
    this->children3 = child3;
    this->childrenList2 = childList2;
}
Node::Node(unsigned line, string m_name,Node* child1, string strval, NodeList* childList1, Node* child2, NodeList* childList2){
    this->line = line;
    this->name = m_name;
    this->children1 = child1;
    this->strval = strval;
    this->childrenList1 = childList1;
    this->children2 = child2;
    this->childrenList2 = childList2;
}
NodeList::NodeList(unsigned line, string type, Node* node){
    this->line = line;
    this->listType = type;
    this->list.push_back(node);
}
void NodeList::addNode(Node* node){
    this->list.push_back(node);
}
string NodeList::printList(){
    if(this->list.size() == 1) return this->list[0]->print();
    string result = "("+(this->listType)+"["+to_string(this->line)+"]";
    for(auto listElem : list) result += listElem->print();
    return result+")";
}
bool NodeList::hasOneElement(){
    return this->list.size() == 1;
}
Node* NodeList::giveMeOnlyElem(){
    return this->list[0];
}