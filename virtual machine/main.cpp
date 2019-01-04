#include <iostream>
#include <vector>
#include <cstdio>
#include <string>
#include <fstream>
#include <map>
#include <clocale>
#include <typeinfo>
enum Instruction {
    START, //pseudoinstrukcja
    EXPRESSION, END_EXPRESSION, BOUNDED_EXPRESSION, END_BOUNDED_EXPRESSION, OR,
    FUNCTION, RETURN, RETURN_VAL, YIELD,
    JUMP_IF_SUCCESS, JUMP_IF_FAIL, JUMP,
    DECLARE_INT, DECLARE_FLOAT, CONCAT, DECLARE_OBJECT,
    PUT_LOCAL, PUT_OBJECT, PUT_CONST_INT,
    PUT_CONST_FLOAT, POP,
    HALT,
    NOT, LIMIT_MARK, LIMIT_UNMARK,
    CALL, ASSIGN, LOAD, STORE,
    BIT_OR, BIT_XOR, BIT_AND,
    EQUAL, NOT_EQUAL, GREATER, LESS, GREATER_EQUAL, LESS_EQUAL,
    EQUAL_FLOAT, NOT_EQUAL_FLOAT, GREATER_FLOAT, LESS_FLOAT, GREATER_EQUAL_FLOAT, LESS_EQUAL_FLOAT,
    BIT_LSH, BIT_RSH,
    ADD_INT, SUB_INT, ADD_FLOAT, SUB_FLOAT,
    MUL_INT, DIV_INT, MOD_INT, MUL_FLOAT, DIV_FLOAT, MOD_FLOAT,
    BIT_NOT,
    MINUS_INT, MINUS_FLOAT,
    INC, DEC,
    I2F, F2I, //tpe conversion
    CALL_BREAKPOINT, COPY, LENGTH, CALL_READ_INT, CALL_READ_CHAR, CALL_PRINT_INT, CALL_PRINT_CHAR //build-in functions
};
union Datatype{
    long long vint;
    double vfloat;
    long long vobject;
};
struct Var{
    Datatype dat;
    bool isReference = false;
};
class object{
public:
    unsigned int length;
    bool hasRef = false;
    Var* variables;
    object(unsigned int i){
        this->length = i;
        if(i>0) variables = new Var[length];
    }
    ~object(){
        delete[] variables;
    }
};
std::map<int,object*> memory; //obiekty
//todo memory dla zmiennych statycznych
unsigned int freeMemoryPointer = 1;
unsigned int freeFunctionPointer = 0;
unsigned int functionPointer;
int instructionRemember = -1;
std::vector< std::pair<Instruction,std::string> > programCode;
std::map<std::string,int> functionTable; //list of functions and its entry points
bool success = 1;//0 - fail, 1 - success
bool debugMode = false;
class expression{
public:
    int bip = -1;
    bool isBounded; //1 = boundedExpression, 0 = expression
    unsigned int instructionStart; //KEY
    bool nextTimeFail = false;
    int instructionEnd = -1;
    int functionPointer = -1; //pointer to function on functionsStack
    int getFunctionPointer();
    bool yieldFlag();
};
class function {
public:
    bool yieldFlag = false;
    std::vector<Var> stack;
    std::map<int,expression*> expressions;
    int currentExpression = -1; //-1 -> nie wskazuje na żaden boundedExpression
    unsigned int instructionPointer = 0;
    std::string name; //KEY
    int callPointer = -1; //wskaźnik na funkcję, która wywołała this
    void deleteExpression(int i);
};
std::map<int,function*> functionStack;
void deleteFunctionFromStack(int i){
    //usuń wszystkie expressiony z mapy expressions
    for(auto it = functionStack[i]->expressions.cbegin(); it != functionStack[i]->expressions.cend(); ++it){
        functionStack[i]->deleteExpression(it->second->getFunctionPointer());
    }
    functionStack.erase(i);
}
int expression::getFunctionPointer(){
    return functionPointer;
}
void function::deleteExpression(int i){
    int d = expressions[i]->getFunctionPointer();
    if(d>=0) deleteFunctionFromStack(d);
    expressions.erase(i);
}
bool expression::yieldFlag(){
    if(functionPointer < 0) return false;
    else return functionStack[functionPointer]->yieldFlag;
}
const unsigned int howOftenGarbageCollector = 10000;
void garbageCollector(){
    std::vector<int> toErase;
    for(auto it = functionStack.cbegin(); it != functionStack.cend(); ++it){
        for(int i=0;i < it->second->stack.size();i++){
            if(it->second->stack[i].isReference) memory[it->second->stack[i].dat.vobject]->hasRef = true;
        }
    }
    for(auto it = memory.cbegin(); it != memory.cend(); ++it){
        for(int i=0;i<it->second->length;i++){
            if(it->second->variables[i].isReference) memory[it->second->variables[i].dat.vobject]->hasRef = true;
        }
    }
    for(auto it = memory.cbegin(); it != memory.cend(); ++it){
        if(it->second->hasRef) it->second->hasRef=false;
        else toErase.push_back(it->first);
    }
    for(int i=0;i<toErase.size();i++){
        memory.erase(toErase[i]);
    }
}
std::string debugger_inst(Instruction instr){
    switch(instr){
        case EXPRESSION: return "EXPRESSION";
        case END_EXPRESSION:
            return "END_EXPRESSION";
        case BOUNDED_EXPRESSION:
            return "BOUNDED_EXPRESSION";
        case END_BOUNDED_EXPRESSION:
            return "END_BOUNDED_EXPRESSION";
        case FUNCTION:
            return "FUNCTION";
        case RETURN:
            return "RETURN";
        case RETURN_VAL:
            return "RETURN_VAL";
        case YIELD:
            return "YIELD";
        case JUMP_IF_SUCCESS:
            return "JUMP_IF_SUCCESS";
        case JUMP_IF_FAIL:
            return "JUMP_IF_FAIL";
        case JUMP:
            return "JUMP";
        case DECLARE_INT: // * -> I
            return "DECLARE_INT";
        case DECLARE_FLOAT: // * -> F
            return "DECLARE_FLOAT";
        case CONCAT: //O, O -> O
            return "CONCAT";
        case DECLARE_OBJECT:
            return "DECLARE_OBJECT";
        case PUT_LOCAL: // * -> I/F
            return "PUT_LOCAL";
        case PUT_OBJECT: // * -> O
            return "PUT_OBJECT";
        case PUT_CONST_INT: // * -> I
            return "PUT_CONST_INT";
        case PUT_CONST_FLOAT: // * -> F
            return "PUT_CONST_FLOAT";
        case POP: // I/F/O -> *
            return "POP";
        case HALT:
            return "HALT";
        case NOT:
            return "NOT";
        case CALL:
            return "CALL";
        case ASSIGN: // I/F/O -> *
            return "ASSIGN";
        case LOAD: // O, I -> I/F/O
            return "LOAD";
        case STORE: // O, I, I/F/O -> *
            return "STORE";
        case BIT_OR: // I, I -> I
            return "BIT_OR";
        case BIT_XOR:
            return "BIT_XOR";
        case BIT_AND:
            return "BIT_AND";
        case EQUAL:
            return "EQUAL";
        case NOT_EQUAL:
            return "NOT_EQUAL";
        case GREATER: // I1, I2 -> * ; I1 > I2
            return "GREATER";
        case LESS:
            return "LESS";
        case GREATER_EQUAL:
            return "GREATER_EQUAL";
        case LESS_EQUAL:
            return "LESS_EQUAL";
        case EQUAL_FLOAT:
            return "EQUAL_FLOAT";
        case NOT_EQUAL_FLOAT:
            return "NOT_EQUAL_FLOAT";
        case GREATER_FLOAT:
            return "GREATER_FLOAT";
        case LESS_FLOAT:
            return "LESS_FLOAT";
        case GREATER_EQUAL_FLOAT:
            return "GREATER_EQUAL_FLOAT";
        case LESS_EQUAL_FLOAT:
            return "LESS_EQUAL_FLOAT";
        case BIT_LSH:
            return "BIT_LSH";
        case BIT_RSH:
            return "BIT_RSH";
        case ADD_INT:
            return "ADD_INT";
        case SUB_INT:
            return "SUB_INT";
        case ADD_FLOAT:
            return "ADD_FLOAT";
        case SUB_FLOAT:
            return "SUB_FLOAT";
        case MUL_INT:
            return "MUL_INT";
        case DIV_INT:
            return "DIV_INT";
        case MOD_INT:
            return "MOD_INT";
        case MUL_FLOAT:
            return "MUL_FLOAT";
        case DIV_FLOAT:
            return "DIV_FLOAT";
        case MOD_FLOAT:
            return "MOD_FLOAT";
        case BIT_NOT:
            return "BIT_NOT";
        case MINUS_INT:
            return "MINUS_INT";
        case MINUS_FLOAT:
            return "MINUS_FLOAT";
        case INC:
            return "INC";
        case DEC:
            return "DEC";
        case I2F:
            return "I2F";
        case F2I:
            return "F2I";
        case CALL_BREAKPOINT:
            return "CALL_BREAKPOINT";
        case COPY: // O -> O
            return "COPY";
        case LENGTH: // O -> I
            return "LENGTH";
        case CALL_READ_INT: // * -> I
            return "CALL_READ_INT";
        case CALL_READ_CHAR: // * -> C
            return "CALL_READ_CHAR";
        case CALL_PRINT_INT: // I -> *
            return "CALL_PRINT_INT";
        case CALL_PRINT_CHAR:
            return "CALL_PRINT_CHAR";
    }
}
void debugger(Instruction instr){
    printf("=========================================\n");
    std::cout<<"Function stack:\n";
    for(auto it = functionStack.cbegin(); it != functionStack.cend(); ++it){
        if(it->first == functionPointer) std::cout<<"->";
        std::cout<<"\t"<<it->first<<" "<<it->second->name<<"\tCE: "<<it->second->currentExpression<<"\n";
        for(auto it2 = functionStack[it->first]->expressions.cbegin(); it2 != functionStack[it->first]->expressions.cend(); ++it2){
            if(it->second->currentExpression == it2->first) std::cout<<"->";
            std::cout<<"\t\t"<<it2->first<<" ";
            if(it2->second->isBounded) std::cout<<"BE";
            else std::cout<<"E";
            std::cout<<"\tFP: "<<it2->second->functionPointer<<"\tNTF: "<<it2->second->nextTimeFail<<"\tYield: "<<it2->second->yieldFlag()<<"\tEOE: "<<it2->second->instructionEnd<<"\n";
        }
    }
    std::cout<<"Memory:\n";
    for(auto it = memory.cbegin(); it != memory.cend(); ++it){
        std::cout<<"\t"<<it->first;
        for(int i=0;i<it->second->length;i++)
            printf("\t%d / %lf",it->second->variables[i].dat.vint,it->second->variables[i].dat.vfloat);
        printf("\n");
    }
    std::cout<<"Instruction Pointer: "<<functionStack[functionPointer]->instructionPointer<<"\nCurrent instruction: "<<debugger_inst(instr)<<"\nSuccess: "<<success<<"\n";
    std::cout<<"Stack of current function:\n";
    for(int i=0;i<functionStack[functionPointer]->stack.size();i++)
        printf("\t%d / %lf \t%s\n",functionStack[functionPointer]->stack[i].dat.vint,functionStack[functionPointer]->stack[i].dat.vfloat,functionStack[functionPointer]->stack[i].isReference ? "reference" : "");
    printf("=========================================\n");
}

int main(int argc, char* argv[]){
    std::string optiond = "-d";
    std::fstream plik;
    std::string sciezka;
    if(argv[1]==optiond && argc==3) plik.open(argv[2], std::ios::in);
    else plik.open(argv[1], std::ios::in);
    if(plik.good()) {
        if(argv[1]==optiond) debugMode = true;
        int n = 0;
        std::string data;
        Instruction i1;
        std::string i2;
        while (!plik.eof()) {
            plik >> data;
            i1 = START;
            i2 = -1;
            if (data == "EXPRESSION") { i1 = EXPRESSION; plik >> i2;}
            if (data == "END_EXPRESSION") { i1 = END_EXPRESSION; plik >> i2;}
            if (data == "BOUNDED_EXPRESSION") { i1 = BOUNDED_EXPRESSION; plik >> i2;}
            if (data == "END_BOUNDED_EXPRESSION") { i1 = END_BOUNDED_EXPRESSION; plik >> i2;}
            //if (data == "OR") { i1 = OR; plik >> i2;}
            if (data == "FUNCTION") {
                i1 = FUNCTION;
                plik >> i2;
                functionTable[i2] = n;
                if(i2=="main"){
                    function* func = new function();
                    func->instructionPointer = n+1;
                    func->name = i2;
                    functionStack[freeFunctionPointer] = func;
                    functionPointer = freeFunctionPointer;
                    freeFunctionPointer++;
                }
                plik >> i2;
            }
            if (data == "RETURN") { i1 = RETURN; }
            if (data == "RETURN_VAL") { i1 = RETURN_VAL; }
            if (data == "YIELD") { i1 = YIELD; }
            if (data == "JUMP_IF_SUCCESS") { i1 = JUMP_IF_SUCCESS; plik >> i2;}
            if (data == "JUMP_IF_FAIL") { i1 = JUMP_IF_FAIL; plik >> i2;}
            if (data == "JUMP") { i1 = JUMP; plik >> i2;} //jumpy mogą skakać tylko w obrębie swojej funkcji
            if (data == "DECLARE_INT") { i1 = DECLARE_INT;}
            if (data == "DECLARE_FLOAT") { i1 = DECLARE_FLOAT;}
            if (data == "CONCAT") { i1 = CONCAT;}
            if (data == "DECLARE_OBJECT") {
                i1 = DECLARE_OBJECT;
                plik >> i2;
                if(stoi(i2)<=0){
                    std::cout << "Error! Could not declare an object with size less than 0" << std::endl;
                    return -1;
                }
            }
            if (data == "PUT_LOCAL") { i1 = PUT_LOCAL; plik >> i2;}
            if (data == "PUT_OBJECT") { i1 = PUT_OBJECT; plik >> i2;}
            if (data == "PUT_CONST_INT") { i1 = PUT_CONST_INT; plik >> i2;}
            if (data == "PUT_CONST_FLOAT") { i1 = PUT_CONST_FLOAT; plik >> i2;}
            if (data == "POP") { i1 = POP; }
            if (data == "HALT") { i1 = HALT; }
            if (data == "NOT") { i1 = NOT; }
            if (data == "LIMIT_MARK") { i1 = LIMIT_MARK; }
            if (data == "LIMIT_UNMARK") { i1 = LIMIT_UNMARK; }
            if (data == "CALL") { i1 = CALL; plik >> i2;}
            if (data == "ASSIGN"){ i1 = ASSIGN; plik >> i2;}
            if (data == "LOAD"){ i1 = LOAD;}
            if (data == "STORE"){ i1 = STORE;}
            if (data == "BIT_OR") { i1 = BIT_OR; }
            if (data == "BIT_XOR") { i1 = BIT_XOR; }
            if (data == "BIT_AND") { i1 = BIT_AND; }
            if (data == "EQUAL") { i1 = EQUAL; }
            if (data == "NOT_EQUAL") { i1 = NOT_EQUAL; }
            if (data == "GREATER") { i1 = GREATER; }
            if (data == "LESS") { i1 = LESS; }
            if (data == "GREATER_EQUAL") { i1 = GREATER_EQUAL; }
            if (data == "LESS_EQUAL") { i1 = LESS_EQUAL; }
            if (data == "EQUAL_FLOAT") { i1 = EQUAL_FLOAT; }
            if (data == "NOT_EQUAL_FLOAT") { i1 = NOT_EQUAL_FLOAT; }
            if (data == "GREATER_FLOAT") { i1 = GREATER_FLOAT; }
            if (data == "LESS_FLOAT") { i1 = LESS_FLOAT; }
            if (data == "GREATER_EQUAL_FLOAT") { i1 = GREATER_EQUAL_FLOAT; }
            if (data == "LESS_EQUAL_FLOAT") { i1 = LESS_EQUAL_FLOAT; }
            if (data == "BIT_LSH") { i1 = BIT_LSH; }
            if (data == "BIT_RSH") { i1 = BIT_RSH; }
            if (data == "ADD_INT") { i1 = ADD_INT; }
            if (data == "SUB_INT") { i1 = SUB_INT; }
            if (data == "ADD_FLOAT") { i1 = ADD_FLOAT; }
            if (data == "SUB_FLOAT") { i1 = SUB_FLOAT; }
            if (data == "MUL_INT") { i1 = MUL_INT; }
            if (data == "DIV_INT") { i1 = DIV_INT; }
            if (data == "MOD_INT") { i1 = MOD_INT; }
            if (data == "MUL_FLOAT") { i1 = MUL_FLOAT; }
            if (data == "DIV_FLOAT") { i1 = DIV_FLOAT; }
            if (data == "MOD_FLOAT") { i1 = MOD_FLOAT; }
            if (data == "BIT_NOT") { i1 = BIT_NOT; }
            if (data == "MINUS_INT") { i1 = MINUS_INT; }
            if (data == "MINUS_FLOAT") { i1 = MINUS_FLOAT; }
            if (data == "INC") { i1 = INC; }
            if (data == "DEC") { i1 = DEC; }
            if (data == "I2F") { i1 = I2F; }
            if (data == "F2I") { i1 = F2I; }
            if (data == "CALL_BREAKPOINT") { i1 = CALL_BREAKPOINT; }
            if (data == "COPY") { i1 = COPY; }
            if (data == "LENGTH") { i1 = LENGTH; }
            if (data == "CALL_READ_INT") { i1 = CALL_READ_INT; }
            if (data == "CALL_READ_CHAR") { i1 = CALL_READ_CHAR; }
            if (data == "CALL_PRINT_INT") { i1 = CALL_PRINT_INT; }
            if (data == "CALL_PRINT_CHAR") { i1 = CALL_PRINT_CHAR; }
            if (i1 == START) {
                std::cout << "Error. No such instruction in line " << n << std::endl;
                return -1;
            }
            programCode.push_back(make_pair(i1, i2));
            n++;
        }
        plik.close();
    }else{
        std::cout << "Error! Could not open the file!" << std::endl;
        return -1;
    }
    setlocale(LC_ALL, "");
    Instruction instr = START;
    Var a,b,c;
    int d, i, oldIP, iteration = 1;
    object* o;
    do{
        instr = programCode[functionStack[functionPointer]->instructionPointer].first;
        if(debugMode) debugger(instr);
        if(iteration % howOftenGarbageCollector == 0) garbageCollector();
        iteration++;
        switch(instr){
            case EXPRESSION: //todo jeżeli mamy nexttimefail skocz na koniec bounded_expression
                success= true;
                d = stoi(programCode[functionStack[functionPointer]->instructionPointer].second);
                if(functionStack[functionPointer]->expressions.count(d) == 0){ //tworzymy BOUNDED_EXPRESSION
                    expression* ex = new expression();
                    ex->isBounded = true;
                    ex->instructionStart = ex->bip = functionStack[functionPointer]->instructionPointer;
                    functionStack[functionPointer]->expressions[d] = ex;
                }
                functionStack[functionPointer]->currentExpression = d;
                functionStack[functionPointer]->instructionPointer = functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->bip + 1;
                break;
            case END_EXPRESSION:
                functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->instructionEnd = functionStack[functionPointer]->instructionPointer;
                if(functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->nextTimeFail){ //takie samo jak else else else
                    success = false;
                    functionStack[functionPointer]->deleteExpression(functionStack[functionPointer]->currentExpression);
                    functionStack[functionPointer]->currentExpression = -1;
                    functionStack[functionPointer]->instructionPointer++;
                }
                else{
                    if(functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->yieldFlag()){
                        if(success){ //yield & success - instructionPointer++, zachowaj stan generatora
                            functionStack[functionPointer]->instructionPointer++;
                            functionStack[functionPointer]->currentExpression = -1;
                        }
                        else{ //yield & fail - skocz BOUNDED_EXPRESSION, zachowaj stan generatora
                            functionStack[functionPointer]->instructionPointer = functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->instructionStart;
                        }
                    }
                    else{
                        if(success){ //nie było yield & success
                            functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->nextTimeFail = true;
                            functionStack[functionPointer]->currentExpression = -1;
                            functionStack[functionPointer]->instructionPointer++;
                        }
                        else{ //nie było yield & fail
                            success = false;
                            functionStack[functionPointer]->deleteExpression(functionStack[functionPointer]->currentExpression);
                            functionStack[functionPointer]->currentExpression = -1;
                            functionStack[functionPointer]->instructionPointer++;
                        }
                    }
                }
                break;
            case BOUNDED_EXPRESSION:
                success= true;
                d = stoi(programCode[functionStack[functionPointer]->instructionPointer].second);
                if(functionStack[functionPointer]->expressions.count(d) == 0){
                    expression* ex = new expression();
                    ex->isBounded = false;
                    ex->instructionStart = functionStack[functionPointer]->instructionPointer;
                    functionStack[functionPointer]->currentExpression = d;
                    functionStack[functionPointer]->expressions[d] = ex;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case END_BOUNDED_EXPRESSION:
                if(functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->nextTimeFail){
                    success = false;
                    functionStack[functionPointer]->deleteExpression(functionStack[functionPointer]->currentExpression);
                    functionStack[functionPointer]->currentExpression = -1;
                    functionStack[functionPointer]->instructionPointer++;
                }
                else{ //yield AND fail - zacznij liczyć od początku (oczywiście generator reaktywuj od yield)
                    if(functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->yieldFlag() && !success){
                        functionStack[functionPointer]->instructionPointer = functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->instructionStart;
                    }
                    else{ //(yield AND success) OR nie było yield - nast. razem liczysz od początku (w tym wszystkie generatory od początku)
                        functionStack[functionPointer]->deleteExpression(functionStack[functionPointer]->currentExpression);
                        functionStack[functionPointer]->currentExpression = -1;
                        functionStack[functionPointer]->instructionPointer++;
                    }
                }
                break;
            /*case OR:
                d = stoi(programCode[functionStack[functionPointer]->instructionPointer].second);
                if(functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->yieldFlag()){
                    if(success){ //yield & success
                        functionStack[functionPointer]->instructionPointer = d;
                    }
                    else{ //yield & fail - skocz bip
                        functionStack[functionPointer]->instructionPointer = functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->bip;
                    }
                }
                else{ //nie było yield
                    functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->bip = functionStack[functionPointer]->instructionPointer + 1;
                    if(success) functionStack[functionPointer]->instructionPointer = d;
                    else functionStack[functionPointer]->instructionPointer++;
                }
                break;*/
            case FUNCTION: //UWAGA! Argumenty funkcji są odczytywane od prawej
                d = stoi(programCode[functionStack[functionPointer]->instructionPointer].second);
                i = functionStack[functionPointer]->instructionPointer;
                oldIP = functionPointer;
                functionStack[functionPointer]->instructionPointer = instructionRemember;
                if(functionStack[functionPointer]->currentExpression >= 0 &&
                   functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->functionPointer >= 0){ //WZNAWIAMY WYKONANIE FUNKCJI
                    functionPointer = functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->functionPointer;
                }
                else{
                    if(functionStack[functionPointer]->currentExpression >= 0){ //JESTEŚMY W BOUNDED EXPRESSION
                        functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->functionPointer = freeFunctionPointer;
                    }
                    function* func = new function();
                    func->instructionPointer = ++i;
                    func->callPointer = functionPointer;
                    func->name = programCode[functionStack[functionPointer]->instructionPointer-1].second;
                    while(d > 0){//func.stack = przepisz argumenty
                        func->stack.push_back(functionStack[functionPointer]->stack.back());
                        functionStack[functionPointer]->stack.pop_back();
                        d--;
                    }
                    functionStack[freeFunctionPointer] = func;
                    functionPointer = freeFunctionPointer;
                    freeFunctionPointer++;
                }
                while(d > 0){//usuń argumenty ze stack
                    functionStack[oldIP]->stack.pop_back();
                    d--;
                }
                break;
            case RETURN:
                d = functionStack[functionPointer]->callPointer;
                deleteFunctionFromStack(functionPointer);
                functionPointer = d;
                if(functionStack[functionPointer]->currentExpression >= 0)
                    functionStack[functionPointer]->expressions[functionStack[functionPointer]->currentExpression]->functionPointer = -1;
                break;
            case RETURN_VAL:
                a = functionStack[functionPointer]->stack.back();
                functionStack[functionPointer]->stack.pop_back();
                functionStack[functionPointer]->yieldFlag = false;
                d = functionStack[functionPointer]->callPointer;
                deleteFunctionFromStack(functionPointer);
                functionPointer = d;
                functionStack[functionPointer]->stack.push_back(a);
                break;
            case YIELD:
                a = functionStack[functionPointer]->stack.back();
                functionStack[functionPointer]->stack.pop_back();
                functionStack[functionPointer]->yieldFlag = true;
                functionStack[functionPointer]->instructionPointer++;
                functionPointer = functionStack[functionPointer]->callPointer;
                functionStack[functionPointer]->stack.push_back(a);
                success = true;
                break;
            case JUMP_IF_SUCCESS:
                if(success) functionStack[functionPointer]->instructionPointer = stoi(programCode[functionStack[functionPointer]->instructionPointer].second);
                else functionStack[functionPointer]->instructionPointer++;
                break;
            case JUMP_IF_FAIL:
                if(!success) functionStack[functionPointer]->instructionPointer = stoi(programCode[functionStack[functionPointer]->instructionPointer].second);
                else functionStack[functionPointer]->instructionPointer++;
                break;
            case JUMP:
                functionStack[functionPointer]->instructionPointer = stoi(programCode[functionStack[functionPointer]->instructionPointer].second);
                break;
            case DECLARE_INT: // * -> I
                a.dat.vint = 0;
                a.isReference = false;
                functionStack[functionPointer]->stack.push_back(a);
                functionStack[functionPointer]->instructionPointer++;
                break;
            case DECLARE_FLOAT: // * -> F
                a.dat.vfloat = 0.0;
                a.isReference = false;
                functionStack[functionPointer]->stack.push_back(a);
                functionStack[functionPointer]->instructionPointer++;
                break;
            case CONCAT: //O, O -> O
                if(success){
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    d = memory[a.dat.vobject]->length + memory[b.dat.vobject]->length;
                    o = new object(d);
                    for(int i=0;i<memory[a.dat.vobject]->length;i++)
                        o->variables[i] = memory[b.dat.vobject]->variables[i];
                    d = memory[a.dat.vobject]->length;
                    for(int i=0;i<d+memory[b.dat.vobject]->length;i++)
                        o->variables[i+d] = memory[a.dat.vobject]->variables[i];
                    memory[freeMemoryPointer] = o;
                    a.dat.vobject = freeMemoryPointer;
                    a.isReference = true;
                    freeMemoryPointer++;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case DECLARE_OBJECT:
                o = new object(stoi(programCode[functionStack[functionPointer]->instructionPointer].second));
                memory[freeMemoryPointer] = o;
                a.dat.vobject = freeMemoryPointer;
                a.isReference = true;
                freeMemoryPointer++;
                functionStack[functionPointer]->stack.push_back(a);
                functionStack[functionPointer]->instructionPointer++;
                break;
            case PUT_LOCAL: // * -> I/F
                if(success){
                    functionStack[functionPointer]->stack.push_back(
                            functionStack[functionPointer]->stack[stoi(programCode[functionStack[functionPointer]->instructionPointer].second)]);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case PUT_OBJECT: // * -> O
                if(success){
                    functionStack[functionPointer]->stack.push_back(
                            functionStack[functionPointer]->stack[stoi(programCode[functionStack[functionPointer]->instructionPointer].second)]);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case PUT_CONST_INT: // * -> I
                if(success) {
                    a.dat.vint = stoi(programCode[functionStack[functionPointer]->instructionPointer].second);
                    a.isReference = false;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case PUT_CONST_FLOAT: // * -> F
                if(success) {
                    a.dat.vfloat = stod(programCode[functionStack[functionPointer]->instructionPointer].second);
                    a.isReference = false;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case POP: // I/F/O -> *
                if(success) {
                    functionStack[functionPointer]->stack.pop_back();
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case HALT:
                break;
            case NOT:
                if(success==0) success=1;
                if(success==1) success=0;
                functionStack[functionPointer]->instructionPointer++;
                break;
            case CALL:
                if(success) {
                    instructionRemember = functionStack[functionPointer]->instructionPointer + 1;
                    functionStack[functionPointer]->instructionPointer = functionTable[programCode[functionStack[functionPointer]->instructionPointer].second];
                }
                break;
            case ASSIGN: // I/F/O -> *
                if(success){
                    functionStack[functionPointer]->stack[stoi(programCode[functionStack[functionPointer]->instructionPointer].second)] = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case LOAD: // O, I -> I/F/O
                if(success) {
                    a = functionStack[functionPointer]->stack.back(); //index
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back(); //object_ref
                    functionStack[functionPointer]->stack.pop_back();
                    if (b.dat.vobject == 0 || b.dat.vobject > memory.size() ||
                        a.dat.vint >= memory[b.dat.vobject]->length || a.dat.vint < 0)
                        success = false;
                    else functionStack[functionPointer]->stack.push_back(memory[b.dat.vobject]->variables[a.dat.vint]);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case STORE: // O, I, I/F/O -> *
                if(success) {
                    a = functionStack[functionPointer]->stack.back(); //value
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back(); //index
                    functionStack[functionPointer]->stack.pop_back();
                    c = functionStack[functionPointer]->stack.back(); //object_ref
                    functionStack[functionPointer]->stack.pop_back();
                    if (c.dat.vobject == 0 || c.dat.vobject > memory.size() ||
                        b.dat.vint >= memory[c.dat.vobject]->length || b.dat.vint < 0)
                        success = false;
                    else memory[c.dat.vobject]->variables[b.dat.vint] = a;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case BIT_OR: // I, I -> I
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    a.dat.vint |= b.dat.vint;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case BIT_XOR:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    a.dat.vint ^= b.dat.vint;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case BIT_AND:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    a.dat.vint &= b.dat.vint;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case EQUAL:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (a.dat.vint == b.dat.vint) success = true;
                    else success = false;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case NOT_EQUAL:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (a.dat.vint != b.dat.vint) success = true;
                    else success = false;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case GREATER: // I1, I2 -> * ; I1 > I2
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (b.dat.vint > a.dat.vint) success = true;
                    else success = false;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case LESS:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (b.dat.vint < a.dat.vint) success = true;
                    else success = false;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case GREATER_EQUAL:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (b.dat.vint >= a.dat.vint) success = true;
                    else success = false;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case LESS_EQUAL:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (b.dat.vint <= a.dat.vint) success = true;
                    else success = false;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case EQUAL_FLOAT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (a.dat.vfloat == b.dat.vfloat) success = true;
                    else success = false;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case NOT_EQUAL_FLOAT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (a.dat.vfloat != b.dat.vfloat) success = true;
                    else success = false;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case GREATER_FLOAT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (b.dat.vfloat > a.dat.vfloat) success = true;
                    else success = false;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
                break;
            case LESS_FLOAT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (b.dat.vfloat < a.dat.vfloat) success = true;
                    else success = false;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case GREATER_EQUAL_FLOAT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (b.dat.vfloat >= a.dat.vfloat) success = true;
                    else success = false;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case LESS_EQUAL_FLOAT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (b.dat.vfloat <= a.dat.vfloat) success = true;
                    else success = false;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case BIT_LSH:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b.dat.vint <<= a.dat.vint;
                    functionStack[functionPointer]->stack.push_back(b);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case BIT_RSH:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b.dat.vint >>= a.dat.vint;
                    functionStack[functionPointer]->stack.push_back(b);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case ADD_INT:
                if(success){
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    a.dat.vint += b.dat.vint;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case SUB_INT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b.dat.vint -= a.dat.vint;
                    functionStack[functionPointer]->stack.push_back(b);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case ADD_FLOAT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    a.dat.vfloat += b.dat.vfloat;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case SUB_FLOAT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b.dat.vfloat -= a.dat.vfloat;
                    functionStack[functionPointer]->stack.push_back(b);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case MUL_INT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b.dat.vint *= a.dat.vint;
                    functionStack[functionPointer]->stack.push_back(b);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case DIV_INT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (a.dat.vint == 0) success = false;//div by 0 -> fail
                    else {
                        b.dat.vint /= a.dat.vint;
                        functionStack[functionPointer]->stack.push_back(b);
                    }
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case MOD_INT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    if (a.dat.vint == 0) success = false;//mod by 0 -> fail
                    else {
                        b.dat.vint %= a.dat.vint;
                        functionStack[functionPointer]->stack.push_back(b);
                    }
                    functionStack[functionPointer]->stack.push_back(b);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case MUL_FLOAT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b.dat.vfloat *= a.dat.vfloat;
                    functionStack[functionPointer]->stack.push_back(b);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case DIV_FLOAT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b.dat.vfloat /= a.dat.vfloat;
                    functionStack[functionPointer]->stack.push_back(b);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case MOD_FLOAT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    b.dat.vfloat -= (int) (b.dat.vfloat / a.dat.vfloat) * a.dat.vfloat;
                    functionStack[functionPointer]->stack.push_back(b);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case BIT_NOT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    a.dat.vint = ~a.dat.vint;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case MINUS_INT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    a.dat.vint = -a.dat.vint;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case MINUS_FLOAT:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    a.dat.vfloat = -a.dat.vfloat;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case INC:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    a.dat.vint++;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case DEC:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    a.dat.vint--;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case I2F:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    a.dat.vfloat = (double) a.dat.vint;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case F2I:
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    a.dat.vint = (long long) a.dat.vfloat;
                    functionStack[functionPointer]->stack.push_back(a);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case CALL_BREAKPOINT:
                debugger(instr);
                getchar();
                break;
            case COPY: // O -> O
                a = functionStack[functionPointer]->stack.back();
                functionStack[functionPointer]->stack.pop_back();
                b.dat.vobject = freeMemoryPointer;
                functionStack[functionPointer]->stack.push_back(b);
                memory[freeMemoryPointer] = memory[a.dat.vobject];
                freeMemoryPointer++;
                functionStack[functionPointer]->instructionPointer++;
                break;
            case LENGTH: // O -> I
                a = functionStack[functionPointer]->stack.back();
                functionStack[functionPointer]->stack.pop_back();
                a.dat.vint = memory[a.dat.vobject]->length;
                functionStack[functionPointer]->stack.push_back(a);
                functionStack[functionPointer]->instructionPointer++;
                break;
            case CALL_READ_INT: // * -> I
                std::cin >> a.dat.vint;
                a.isReference = false;
                functionStack[functionPointer]->stack.push_back(a);
                functionStack[functionPointer]->instructionPointer++;
                break;
            case CALL_READ_CHAR: // * -> C
                a.dat.vint = getchar();
                a.isReference = false;
                functionStack[functionPointer]->stack.push_back(a);
                functionStack[functionPointer]->instructionPointer++;
                break;
            case CALL_PRINT_INT: // I -> *
                if(success) {
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    printf("%lld", a.dat.vint);
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            case CALL_PRINT_CHAR: // C -> *
                if(success){
                    a = functionStack[functionPointer]->stack.back();
                    functionStack[functionPointer]->stack.pop_back();
                    std::wcout<<(wchar_t) a.dat.vint;
                }
                functionStack[functionPointer]->instructionPointer++;
                break;
            default:
                std::cout<<"Error, no such instruction"<<std::endl;
                return -1;
        }
    }while(programCode[functionStack[functionPointer]->instructionPointer].first != HALT);
    if(debugMode) std::cout<<"Execution of program had been succeed";
    return 0;
}