#include "node.hpp"
using namespace std;

vector<Node*> functions;
vector<Node*> operatorOverloaded;
vector<Node*> structures;
vector<Node*> datas;
void semanticAnalyzerMain(Node* node){

}

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