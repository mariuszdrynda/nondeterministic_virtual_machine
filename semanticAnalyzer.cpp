#include "node.hpp"
using namespace std;

vector<Node*> functions;
vector<Node*> operatorOverloaded;
vector<Node*> structures;
vector<Node*> datas;

Node* findEntryPoint(NodeList* programList){
    //TODO: find function of name main
    //main can't be override
    //main has return Void
    //main can has no arguments or has [String] (this list could be empty)
    //add main to functions
    return programList[0];
}
void giveTypeToAllNodes(Node* node){
/*TODO:
    - check all types, all nodes (and nodeList) have to have types
    -- for functions it's type they return
    -- for nodeList it's type of last node
    -- for literals it's obvius
    - divide programList into
    -- structs
    -- datas
    -- functions
    -- operator overloading
*/
    /*BIG SWITCH*/
}
void semanticAnalyzerMain(NodeList* programList){
    Node* entryPoint = findEntryPoint(programList);
    giveTypeToAllNodes(entryPoint);
}