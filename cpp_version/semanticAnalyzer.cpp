#include <stdlib.h>
#include "ast.hpp"

using ProgramStructure = std::tuple<std::vector<std::shared_ptr<Function>>, 
    std::vector<std::shared_ptr<Function>>, 
    std::vector<std::shared_ptr<Struct>>, 
    std::vector<std::shared_ptr<Struct>>>;
ProgramStructure divideProgram(std::shared_ptr<NodeList> programList){
    std::vector<std::shared_ptr<Function>> functions;
    std::vector<std::shared_ptr<Function>> operatorOverloaded;
    std::vector<std::shared_ptr<Struct>> structures;
    std::vector<std::shared_ptr<Struct>> datas;
    /*TODO
    divide programList into
    -- structs
    -- datas
    -- functions
    -- operator overloading
    */
   return std::make_tuple(functions, operatorOverloaded, structures, datas);
}
void giveTypeToAllNodes(std::shared_ptr<AST> node){
/*TODO:
    - check all types, all nodes (and nodeList) have to have types
    -- for functions it's type they return
    -- for nodeList it's type of last node
    -- for literals it's obvius
    -- matching functions to their calls
*/
}
void semanticAnalyzerMain(std::shared_ptr<NodeList> programList){
    std::shared_ptr<Function> entryPoint = programList->findMainFunction();
    giveTypeToAllNodes(entryPoint);
    // ProgramStructure programStructure = divideProgram(programList);
}