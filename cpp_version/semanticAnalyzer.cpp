#include <stdlib.h>
#include "ast/ast.hpp"

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
void semanticAnalyzerMain(std::shared_ptr<NodeList> programList){
    SemanticAnalyzerHelper sah(programList->getIDs());
    programList->findMainFunction()->setInnerType(sah);
    std::cout<<"\tSEMANTIC ANALYZER MAIN ENDED WORK!\n";
    // ProgramStructure programStructure = divideProgram(programList);
}