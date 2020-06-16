#include <utility>
#include <vector>
#include <map>
#include <iostream>
#include <memory>
#include <utility>
#include <mutex>
#include <future>
#include <thread>
#include "stackFrame.hpp"
using namespace std;

bool debug = false;

// mutex io;

auto in(string searched, map<string,shared_ptr<StackFrame>> mem) -> bool{
    auto it = mem.find(searched);
    if (it != mem.end()) return true;
    else return false;
}
auto in(string searched, map<std::string, std::function<std::shared_ptr<StackFrame>(std::shared_ptr<StackFrame>)>> globalMem) -> bool{
    auto it = globalMem.find(searched);
    if (it != globalMem.end()) return true;
    else return false;
}

auto prepareToExecute(std::vector<std::vector<string>> ir) -> pair<unsigned, map<string,shared_ptr<StackFrame>>>{
    map<string,shared_ptr<StackFrame>> globalMem;
    unsigned ip = 0;
    while(ip < ir.size()){
        if(ir[ip][0] == "FUN"){
            if(in(ir[ip][1], globalMem)) throw "Reused name "+ir[ip][1]+" in global scope.";
            globalMem[ir[ip][1]] = make_shared<FunctionLiteral>(ip);
        }
        if(ir[ip][0] == "STRUCT"){
            if(in(ir[ip][1], globalMem)) throw "Reused name "+ir[ip][1]+" in global scope.";
            globalMem[ir[ip][1]] = make_shared<StructLiteral>(ip);
        }
        ++ip;
    }
    if(in("main", globalMem)) ip = globalMem["main"]->getLine();
    else throw "No main function.";
    return make_pair(ip, globalMem);
}

auto deepc(vector<map<string,shared_ptr<StackFrame>>> memory) -> vector<map<string,shared_ptr<StackFrame>>>{
    vector<map<string,shared_ptr<StackFrame>>> ret;
    for(int i = 0; i < memory.size(); ++i){
        ret.push_back(std::map<std::string,shared_ptr<StackFrame>>());
        map<string,shared_ptr<StackFrame>>::iterator it = memory[i].begin();
        while (it != memory[i].end()){
            std::string word = it->first;
            std::shared_ptr<StackFrame> copied= it->second->clone();
            ret[i][word] = copied;
            it++;
        }
    }
    return ret;
}

auto deepc(vector<shared_ptr<StackFrame>> stack) -> vector<shared_ptr<StackFrame>>{
    vector<shared_ptr<StackFrame>> ret;
    ret.reserve(stack.size());
    for(auto& i : stack){
        ret.push_back(i->clone());
    }
    return ret;
}

auto execute(std::vector<std::vector<std::string>> ir,
             unsigned ip,
             std::map<string,std::shared_ptr<StackFrame>> globalMem,
             std::vector<std::map<std::string,shared_ptr<StackFrame>>> memory,
             std::vector<std::shared_ptr<StackFrame>> stack,
             std::shared_ptr<StackFrame> onstack,
             unsigned deep) -> pair<unsigned, shared_ptr<StackFrame>>;
auto makeNondeterminism(std::vector<std::vector<std::string>> ir,
    unsigned ip,
    std::map<string,std::shared_ptr<StackFrame>> globalMem,
    vector<map<string,shared_ptr<StackFrame>>> memory,
    vector<shared_ptr<StackFrame>> stack,
    unsigned deep) -> unsigned{
    if(stack.back()->getType() == StackFrameType::YIELD_BUFFER){
        auto buffer = static_pointer_cast<YieldBuffer>(stack.back())->list;
        stack.pop_back();
        int retip = 0;
        vector<future<pair<unsigned, shared_ptr<StackFrame>>>> results;
        // vector<pair<unsigned, shared_ptr<StackFrame>>> results;
        for (auto& b : buffer) {
            future<pair<unsigned, shared_ptr<StackFrame>>> f = async(execute, ir, ip, globalMem, deepc(memory), deepc(stack), b, deep+1);
            // pair<unsigned, shared_ptr<StackFrame>> f = execute(ir, ip, globalMem, deepc(memory), deepc(stack), b, deep+1);
            results.push_back(move(f));
        }
        for (auto& f : results) {
            f.wait();
        }
        vector<shared_ptr<StackFrame>> all;
        for(auto& res : results){
            pair<unsigned, shared_ptr<StackFrame>> a = res.get();
            // pair<unsigned, shared_ptr<StackFrame>> a = res;
            retip = a.first > retip ? a.first : retip;
            all.push_back(a.second);
        }
        while(!(stack.back()->getType() == StackFrameType::EVERY_BUFFER)) stack.pop_back();
        stack.pop_back();
        stack.push_back(make_shared<ListFrame>(make_shared<TupleFrame>(all)));
        return retip;
    } else {
        throw "Error during splitting execution paths.";
    }
}

auto execute(std::vector<std::vector<std::string>> ir,
             unsigned ip,
             std::map<string,std::shared_ptr<StackFrame>> globalMem,
             std::vector<std::map<std::string,shared_ptr<StackFrame>>> memory,
             std::vector<std::shared_ptr<StackFrame>> stack,
             std::shared_ptr<StackFrame> onstack,
             unsigned deep) -> pair<unsigned, shared_ptr<StackFrame>>{
    if(onstack->getType() != StackFrameType::NULL_FRAME) stack.push_back(onstack);
    while(true){
        auto currentInstr = ir[ip];
        if(debug){
            io.lock();
            cout << "======== DEBUGGER ========\nGLOBAL MEMORY [";
            for(auto it = globalMem.cbegin(); it != globalMem.cend(); ++it){
                std::cout << it->first << " " << it->second->print() << ", ";
            }
            cout << "]\nMEMORY [";
            for(auto& memoryOfFunction : memory){
                cout<<"{";
                for(auto it = memoryOfFunction.cbegin(); it != memoryOfFunction.cend(); ++it){
                    std::cout << it->first << " " << it->second->print() << ", ";
                }
                cout<<"}, ";
            }
            cout << "]\nSTACK [";
            for(auto& a : stack) std::cout << a->print() << ", ";
            cout << "]\n";
            cout << "IP: " << ip << '\n';
            cout << "DEEP: " << deep << '\n';
            cout << "CURRENT INSTRUCTION: " << currentInstr[0] << '\n';
            // getchar();
            io.unlock();
        }
        if(currentInstr[0] == "SEMICOLON"){
            while (!(stack.back()->getType() == StackFrameType::FUNCTION_FRAME || stack.back()->getType() == StackFrameType::EVERY_BUFFER))
                stack.pop_back();
            ++ip;
        } else if(currentInstr[0] == "FUN"){ //args może być empty lub tuple
            memory.push_back(std::map<std::string,shared_ptr<StackFrame>>());
            if (stack.size() > 0){
                auto args = stack.back();
                stack.pop_back();
                if(args->getType() == StackFrameType::EMPTY_FRAME){
                    if(stoll(currentInstr[2]) != 0)
                        make_shared<FailFrame>();
                }
                else if(args->getType() == StackFrameType::TUPLE_FRAME){
                    auto arg = static_pointer_cast<TupleFrame>(args);
                    if((arg->value).size() != stoll(currentInstr[2]))
                        make_shared<FailFrame>();
                    else{
                        for(unsigned i = 0; i < stoll(currentInstr[2]); ++i){
                            memory.back()[currentInstr[3+i]] = static_pointer_cast<TupleFrame>(args)->value[i];
                        }
                    } 
                }
            }
            stack.push_back(make_shared<FunctionFrame>(currentInstr[1], ip));
            ++ip;
        } else if(currentInstr[0] == "YIELD"){
            bool generator = false;
            unsigned d = 0;
            for(int i = stack.size() - 1; i >= 0; --i){
                if(stack[i]->getType() == StackFrameType::EVERY_BUFFER){
                    generator = true;
                    break;
                } else if(stack[i]->getType() == StackFrameType::FUNCTION_FRAME){
                    ++d;
                    if(d >= 2) break;
                }
            }
            if(generator){
                shared_ptr<StackFrame> toAddToBuffer;
                if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                    toAddToBuffer = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
                } else toAddToBuffer = stack.back();
                stack.pop_back();
                if(in("?YieldBuffer", memory.back())){
                    static_pointer_cast<YieldBuffer>(memory.back()["?YieldBuffer"])->list.push_back(toAddToBuffer);     
                } else {
                    memory.back()["?YieldBuffer"] = make_shared<YieldBuffer>(toAddToBuffer);
                }
            } else {
                shared_ptr<StackFrame> returned;
                if(stack.back()->getType() == StackFrameType::FUNCTION_FRAME){
                    returned = make_shared<VoidFrame>();
                } else if(stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                    string ide = static_pointer_cast<IdentifierFrame>(stack.back())->value;
                    if(in(ide, memory.back())) returned = memory.back()[ide];
                    else returned = make_shared<FailFrame>();
                } else{
                    returned = stack.back();
                }
                memory.pop_back();
                while(stack.back()->getType() != StackFrameType::FUNCTION_FRAME) stack.pop_back();
                if (stack.back()->getType() == StackFrameType::FUNCTION_FRAME){
                    shared_ptr<FunctionFrame> ff = static_pointer_cast<FunctionFrame>(stack.back());
                    if(stack.size() == 1 && ff->name == "main") break; //HALT
                    stack.pop_back();
                    ip = static_pointer_cast<RetFromFunctionFrame>(stack.back())->line+1;
                    stack.pop_back();
                    stack.push_back(returned);
                } else throw"Internal error.";
            }
            ++ip;
        } else if(currentInstr[0] == "ENDFUN" || currentInstr[0] == "RET"){
            shared_ptr<StackFrame> returned;
            if (in("?YieldBuffer", memory.back())){
                shared_ptr<YieldBuffer> ret = static_pointer_cast<YieldBuffer>(memory.back()["?YieldBuffer"]);
                if(ret->list.size() == 1) returned = ret->list[0];
                else returned = ret;
            } else if(stack.back()->getType() == StackFrameType::FUNCTION_FRAME){
                returned = make_shared<VoidFrame>();
            } else if(stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                string ide = static_pointer_cast<IdentifierFrame>(stack.back())->value;
                if(in(ide, memory.back())) returned = memory.back()[ide];
                else returned = make_shared<FailFrame>();
            } else{
                returned = stack.back();
            }
            memory.pop_back();
            while(stack.back()->getType() != StackFrameType::FUNCTION_FRAME) stack.pop_back();
            if (stack.back()->getType() == StackFrameType::FUNCTION_FRAME){
                shared_ptr<FunctionFrame> ff = static_pointer_cast<FunctionFrame>(stack.back());
                if(stack.size() == 1 && ff->name == "main") break; //HALT
                stack.pop_back();
                ip = static_pointer_cast<RetFromFunctionFrame>(stack.back())->line+1;
                stack.pop_back();
                stack.push_back(returned);
                if (stack.back()->getType() == StackFrameType::YIELD_BUFFER){
                    ip = makeNondeterminism(ir, ip, globalMem, memory, stack, deep);
                    if(deep > 0){
                        if(stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME)
                            return make_pair(ip, memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value]);
                        else return make_pair(ip, stack.back());
                    }
                }
            } else throw"Internal error.";
        } else if(currentInstr[0] == "ASSIGN"){
            shared_ptr<StackFrame> assignFrom;
            if(stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                assignFrom = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else assignFrom = stack.back();
            stack.pop_back();
            if(stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                shared_ptr<IdentifierFrame> assignTo = static_pointer_cast<IdentifierFrame>(stack.back());
                stack.pop_back();
                memory.back()[assignTo->value] = assignFrom;
                stack.push_back(assignTo);
            } else if(stack.back()->getType() == StackFrameType::REFFERENCE_ARRAY_ELEM){
                shared_ptr<RefferenceArrayElem> assignTo = static_pointer_cast<RefferenceArrayElem>(stack.back());
                stack.pop_back();
                if(assignTo->left->getType() == StackFrameType::IDENTIFIER_FRAME){
                    shared_ptr<StackFrame> l = memory.back()[static_pointer_cast<IdentifierFrame>(assignTo->left)->value];
                    if(l->getType() == StackFrameType::LIST_FRAME){
                        auto list = static_pointer_cast<ListFrame>(l);
                        list->update(assignFrom, assignTo->index);
                        stack.push_back(assignTo->left);
                    } else {
                        stack.push_back(make_shared<FailFrame>());
                    }
                } else {
                    stack.push_back(make_shared<FailFrame>());
                }
            } else if(stack.back()->getType() == StackFrameType::REFFERENCE_FIELD){
                shared_ptr<RefferenceField> assignTo = static_pointer_cast<RefferenceField>(stack.back());
                stack.pop_back();
                if(assignTo->left->getType() == StackFrameType::IDENTIFIER_FRAME){
                    shared_ptr<StackFrame> l = memory.back()[static_pointer_cast<IdentifierFrame>(assignTo->left)->value];
                    if(l->getType() == StackFrameType::OBJECT_FRAME){
                        auto obj = static_pointer_cast<ObjectFrame>(l);
                        obj->update(assignFrom, assignTo->fieldName);
                        stack.push_back(assignTo->left);
                    } else {
                        stack.push_back(make_shared<FailFrame>());
                    }
                } else {
                    stack.push_back(make_shared<FailFrame>());
                }
            }
            ++ip;
        } else if(currentInstr[0] == "EVERY"){
            stack.push_back(make_shared<EveryBuffer>());
            ++ip;
        } else if(currentInstr[0] == "EVERYBODY" || currentInstr[0] == "NDT_AND"){
            if(stack.back()->getType() == StackFrameType::FAIL_FRAME ||
                (stack.back()->getType() == StackFrameType::BOOL &&
                !(static_pointer_cast<BoolFrame>(stack.back())->value))){
                    ip = stoll(currentInstr[2]);
                } else ++ip;
            while(!(stack.back()->getType() == StackFrameType::EVERY_BUFFER)) stack.pop_back();
        } else if(currentInstr[0] == "ENDEVERY"){
            if(deep > 0){
                if(stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME)
                    return make_pair(ip, memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value]);
                else return make_pair(ip, stack.back());
            } else ++ip;
        } else if(currentInstr[0] == "DOWHILEBODY"){
            ++ip;
        } else if(currentInstr[0] == "DOWHILE"){
            ++ip;
        } else if(currentInstr[0] == "ENDDOWHILE"){
            if(stack.back()->getType() == StackFrameType::BOOL){
                bool condition = static_pointer_cast<BoolFrame>(stack.back())->value;
                stack.pop_back();
                if(condition) ip = stoul(currentInstr[2]);
                else ++ip;
            } else {
                stack.pop_back();
                ++ip;
            }
        } else if(currentInstr[0] == "WHILE"){
            ++ip;
        } else if(currentInstr[0] == "DO" || currentInstr[0] == "THEN"){
            if(stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                auto ide = static_pointer_cast<IdentifierFrame>(stack.back())->value;
                stack.pop_back();
                if(in(ide, memory.back())){
                    auto val = memory.back()[ide];
                    if(val->getType() == StackFrameType::BOOL){
                        bool condition = static_pointer_cast<BoolFrame>(val)->value;
                        if(condition) ++ip;
                        else ip = stoul(currentInstr[2]);
                    } else{
                        ip = stoul(currentInstr[2]);
                    }
                } else{
                    ip = stoul(currentInstr[2]);
                }
            } else if(stack.back()->getType() == StackFrameType::BOOL){
                bool condition = static_pointer_cast<BoolFrame>(stack.back())->value;
                stack.pop_back();
                if(condition) ++ip;
                else ip = stoul(currentInstr[2]);
            } else {
                stack.pop_back();
                ip = stoul(currentInstr[2]);
            }
        } else if(currentInstr[0] == "ENDWHILE"){
            ip = stoul(currentInstr[2]);
        } else if(currentInstr[0] == "IF"){
            ++ip;
        } else if(currentInstr[0] == "ELSE"){
            ip = std::stoul(currentInstr[2]);
        } else if(currentInstr[0] == "ENDIF"){
            ++ip;
        } else if(currentInstr[0] == "BREAK"){
            ip = std::stoul(currentInstr[2]);
        } else if(currentInstr[0] == "CONTINUE"){
            ip = std::stoul(currentInstr[2]);
        } else if(currentInstr[0] == "COMMA"){
            int nrOfElems = std::stoll(currentInstr[1]);
            std::vector<shared_ptr<StackFrame>> vec(nrOfElems, std::make_shared<NullFrame>());
            --nrOfElems;
            for(;nrOfElems >= 0; --nrOfElems){
                vec[nrOfElems] = stack.back();
                if(vec[nrOfElems]->getType() == StackFrameType::IDENTIFIER_FRAME){
                    std::string val = std::static_pointer_cast<IdentifierFrame>(vec[nrOfElems])->value;
                    if(in(val, memory.back())) vec[nrOfElems] = memory.back()[val];
                }
                stack.pop_back();
            } 
            shared_ptr<TupleFrame> result = std::make_shared<TupleFrame>(vec);
            stack.push_back(result);
            ++ip;
        } else if(currentInstr[0] == "NDT_OR"){
//             nrOfElems = -int(currentInstr[1])
//             yb = YieldBuffer()
//             yb.list += stack[nrOfElems:]
//             del stack[nrOfElems:]
//             print(yb)
//             stack.push_back(yb)
//             ip = makeNondeterminism(ir, ip+1, globalMem, memory, stack, deep)
//             if(deep>0):
//                 return ip, (memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1])
            ++ip;
        } else if(currentInstr[0] == "NDT_NOT"){
            if(stack.back()->getType() == StackFrameType::FAIL_FRAME) stack.pop_back();
            else stack.push_back(make_shared<FailFrame>());
            ++ip;
        } else if(currentInstr[0] == "LOG_AND"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::BOOL && right->getType() == StackFrameType::BOOL){
                bool res = (static_pointer_cast<BoolFrame>(left)->value) && (static_pointer_cast<BoolFrame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "LOG_OR"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::BOOL && right->getType() == StackFrameType::BOOL){
                bool res = (static_pointer_cast<BoolFrame>(left)->value) || (static_pointer_cast<BoolFrame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "EQU"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::I64 && right->getType() == StackFrameType::I64){
                bool res = (static_pointer_cast<I64Frame>(left)->value) == (static_pointer_cast<I64Frame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else if(left->getType() == StackFrameType::F64 && right->getType() == StackFrameType::F64){
                bool res = (static_pointer_cast<F64Frame>(left)->value) == (static_pointer_cast<F64Frame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else if(left->getType() == StackFrameType::BOOL && right->getType() == StackFrameType::BOOL){
                bool res = (static_pointer_cast<BoolFrame>(left)->value) == (static_pointer_cast<BoolFrame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "NEQU"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::I64 && right->getType() == StackFrameType::I64){
                bool res = (static_pointer_cast<I64Frame>(left)->value) != (static_pointer_cast<I64Frame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else if(left->getType() == StackFrameType::F64 && right->getType() == StackFrameType::F64){
                bool res = (static_pointer_cast<F64Frame>(left)->value) != (static_pointer_cast<F64Frame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else if(left->getType() == StackFrameType::BOOL && right->getType() == StackFrameType::BOOL){
                bool res = (static_pointer_cast<BoolFrame>(left)->value) != (static_pointer_cast<BoolFrame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "GEQ"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::I64 && right->getType() == StackFrameType::I64){
                bool res = (static_pointer_cast<I64Frame>(left)->value) >= (static_pointer_cast<I64Frame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else if(left->getType() == StackFrameType::F64 && right->getType() == StackFrameType::F64){
                bool res = (static_pointer_cast<F64Frame>(left)->value) >= (static_pointer_cast<F64Frame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "LEQ"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::I64 && right->getType() == StackFrameType::I64){
                bool res = (static_pointer_cast<I64Frame>(left)->value) <= (static_pointer_cast<I64Frame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else if(left->getType() == StackFrameType::F64 && right->getType() == StackFrameType::F64){
                bool res = (static_pointer_cast<F64Frame>(left)->value) <= (static_pointer_cast<F64Frame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "LESS"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::I64 && right->getType() == StackFrameType::I64){
                bool res = (static_pointer_cast<I64Frame>(left)->value) < (static_pointer_cast<I64Frame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else if(left->getType() == StackFrameType::F64 && right->getType() == StackFrameType::F64){
                bool res = (static_pointer_cast<F64Frame>(left)->value) < (static_pointer_cast<F64Frame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "GREATER"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::I64 && right->getType() == StackFrameType::I64){
                bool res = (static_pointer_cast<I64Frame>(left)->value) > (static_pointer_cast<I64Frame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else if(left->getType() == StackFrameType::F64 && right->getType() == StackFrameType::F64){
                bool res = (static_pointer_cast<F64Frame>(left)->value) > (static_pointer_cast<F64Frame>(right)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "SUB"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::I64 && right->getType() == StackFrameType::I64){
                long long res = (static_pointer_cast<I64Frame>(left)->value) - (static_pointer_cast<I64Frame>(right)->value);
                stack.push_back(make_shared<I64Frame>(res));
            } else if(left->getType() == StackFrameType::F64 && right->getType() == StackFrameType::F64){
                double res = (static_pointer_cast<F64Frame>(left)->value) - (static_pointer_cast<F64Frame>(right)->value);
                stack.push_back(make_shared<F64Frame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "ADD"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::I64 && right->getType() == StackFrameType::I64){
                long long res = (static_pointer_cast<I64Frame>(left)->value) + (static_pointer_cast<I64Frame>(right)->value);
                stack.push_back(make_shared<I64Frame>(res));
            } else if(left->getType() == StackFrameType::F64 && right->getType() == StackFrameType::F64){
                double res = (static_pointer_cast<F64Frame>(left)->value) + (static_pointer_cast<F64Frame>(right)->value);
                stack.push_back(make_shared<F64Frame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "MUL"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::I64 && right->getType() == StackFrameType::I64){
                long long res = (static_pointer_cast<I64Frame>(left)->value) * (static_pointer_cast<I64Frame>(right)->value);
                stack.push_back(make_shared<I64Frame>(res));
            } else if(left->getType() == StackFrameType::F64 && right->getType() == StackFrameType::F64){
                double res = (static_pointer_cast<F64Frame>(left)->value) * (static_pointer_cast<F64Frame>(right)->value);
                stack.push_back(make_shared<F64Frame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "DIV"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::I64 && right->getType() == StackFrameType::I64 && static_pointer_cast<I64Frame>(right)->value != 0){
                long long res = (static_pointer_cast<I64Frame>(left)->value) / (static_pointer_cast<I64Frame>(right)->value);
                stack.push_back(make_shared<I64Frame>(res));
            } else if(left->getType() == StackFrameType::F64 && right->getType() == StackFrameType::F64 && static_pointer_cast<I64Frame>(right)->value != 0){
                double res = (static_pointer_cast<F64Frame>(left)->value) / (static_pointer_cast<F64Frame>(right)->value);
                stack.push_back(make_shared<F64Frame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "MOD"){
            shared_ptr<StackFrame> left, right;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                right = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else right = stack.back();
            stack.pop_back();
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::I64 && right->getType() == StackFrameType::I64 && static_pointer_cast<I64Frame>(right)->value != 0){
                long long res = (static_pointer_cast<I64Frame>(left)->value) % (static_pointer_cast<I64Frame>(right)->value);
                stack.push_back(make_shared<I64Frame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "MINUS"){
            shared_ptr<StackFrame> left;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::I64){
                long long res = -(static_pointer_cast<I64Frame>(left)->value);
                stack.push_back(make_shared<I64Frame>(res));
            } else if(left->getType() == StackFrameType::F64){
                double res = -(static_pointer_cast<F64Frame>(left)->value);
                stack.push_back(make_shared<F64Frame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "LOG_NOT"){
            shared_ptr<StackFrame> left;
            if (stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME){
                left = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
            } else left = stack.back();
            stack.pop_back();
            if(left->getType() == StackFrameType::BOOL){
                bool res = !(static_pointer_cast<BoolFrame>(left)->value);
                stack.push_back(make_shared<BoolFrame>(res));
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "ARGUMENT"){
            auto args = stack.back();
            stack.pop_back();
            if(args->getType() == StackFrameType::TUPLE_FRAME){
                auto arg = static_pointer_cast<TupleFrame>(args);
                for(auto& i : arg->value){
                    if (i->getType() == StackFrameType::IDENTIFIER_FRAME)
                        i = memory.back()[static_pointer_cast<IdentifierFrame>(stack.back())->value];
                }
                stack.push_back(arg);
            } else if(args->getType() == StackFrameType::EMPTY_FRAME){
                stack.push_back(args);
            } else if (args->getType() == StackFrameType::IDENTIFIER_FRAME){
                string val = static_pointer_cast<IdentifierFrame>(args)->value;
                shared_ptr<StackFrame> arg;
                if(in(val, memory.back())){
                    arg = memory.back()[val];
                    stack.push_back(make_shared<TupleFrame>(arg));
                }
                else stack.push_back(make_shared<FailFrame>());
            } else {
                shared_ptr<StackFrame> arg = args;
                stack.push_back(make_shared<TupleFrame>(arg));
            }
            ++ip;
        } else if(currentInstr[0] == "CALL"){
            auto args = stack.back();
            stack.pop_back();
            auto callingFunction = stack.back();
            stack.pop_back();
            if(in(callingFunction->getValue(), globalMem)){
                auto nextIp = globalMem[callingFunction->getValue()]->getLine();
                stack.push_back(std::make_shared<RetFromFunctionFrame>(ip));
                ip = nextIp;
                stack.push_back(args);
            } else if(in(callingFunction->getValue(), buildInDict)){
                stack.push_back(buildInDict[callingFunction->getValue()](args));
                ++ip;
            }
            else stack.push_back(std::make_shared<FailFrame>()); //Trying to call undefined function
        } else if(currentInstr[0] == "DOT"){
            auto fieldNameFrame = stack.back();
            stack.pop_back();
            auto objectNameFrame = stack.back();
            stack.pop_back();
            if(fieldNameFrame->getType() == StackFrameType::IDENTIFIER_FRAME &&
                objectNameFrame->getType() == StackFrameType::IDENTIFIER_FRAME){
                    auto fieldName = static_pointer_cast<IdentifierFrame>(fieldNameFrame)->value;
                    auto objectName = static_pointer_cast<IdentifierFrame>(objectNameFrame)->value;
                    if(in(objectName, memory.back()) && memory.back()[objectName]->getType() == StackFrameType::OBJECT_FRAME){
                        auto obj = static_pointer_cast<ObjectFrame>(memory.back()[objectName]);
                        if(obj->in(fieldName)) stack.push_back(obj->str[fieldName]);
                        else stack.push_back(make_shared<FailFrame>());
                    }else {
                        stack.push_back(make_shared<FailFrame>());
                    }
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip; 
        } else if(currentInstr[0] == "REF_FIELD"){
            auto nameOfField = stack.back();
            stack.pop_back();
            auto objectIdent = stack.back();
            stack.pop_back();
            if(nameOfField->getType() == StackFrameType::IDENTIFIER_FRAME){
                string i = static_pointer_cast<IdentifierFrame>(nameOfField)->value;
                if(objectIdent->getType() == StackFrameType::IDENTIFIER_FRAME){
                    stack.push_back(std::make_shared<RefferenceField>(objectIdent, i));
                } else {
                    stack.push_back(std::make_shared<FailFrame>());
                }
            } else {
                stack.push_back(std::make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "OBJECT"){
            auto tup = stack.back();
            stack.pop_back();
            auto structIdent = stack.back();
            stack.pop_back();
            if(structIdent->getType() == StackFrameType::IDENTIFIER_FRAME){
                auto structName = static_pointer_cast<IdentifierFrame>(structIdent)->value;
                if(in(structName, globalMem)){
                    if(tup->getType() == StackFrameType::TUPLE_FRAME){
                        auto tuple = static_pointer_cast<TupleFrame>(tup);
                        if(globalMem[structName]->getType() == StackFrameType::STRUCT_LITERAL){
                            unsigned strLiteral = static_pointer_cast<StructLiteral>(globalMem[structName])->line;
                            int size = tuple->value.size();
                            if(size == stoll(ir[strLiteral][2])){
                                auto dic = map<string, shared_ptr<StackFrame>>();
                                for(int i = 0; i < size; ++i){
                                    dic[ir[strLiteral][i+3]] = tuple->value[i];
                                }
                                stack.push_back(make_shared<ObjectFrame>(structName, dic));
                            } else {
                                stack.push_back(make_shared<FailFrame>());
                            } 
                        } else {
                            stack.push_back(make_shared<FailFrame>());
                        }
                    } else {
                        stack.push_back(make_shared<FailFrame>());
                    }
                } else {
                    stack.push_back(make_shared<FailFrame>());
                }
            } else {
                stack.push_back(make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "REF_ARR_ELEM"){
            auto index = stack.back();
            stack.pop_back();
            auto arrayIdent = stack.back(); //ident or reff_arr_elem
            stack.pop_back();
            if(index->getType() == StackFrameType::I64){
                unsigned i = std::static_pointer_cast<I64Frame>(index)->value;
                if(arrayIdent->getType() == StackFrameType::IDENTIFIER_FRAME){
                    stack.push_back(std::make_shared<RefferenceArrayElem>(arrayIdent, i));
                } else if(arrayIdent->getType() == StackFrameType::REFFERENCE_ARRAY_ELEM){
                    stack.push_back(std::make_shared<RefferenceArrayElem>(arrayIdent, i));
                } else {
                    stack.push_back(std::make_shared<FailFrame>());
                }
            } else {
                stack.push_back(std::make_shared<FailFrame>());
            }
            ++ip;
        } else if(currentInstr[0] == "ARRAY_ELEM"){
            std::shared_ptr<StackFrame> index;
            if(stack.back()->getType() == StackFrameType::IDENTIFIER_FRAME)
                index = memory.back()[std::static_pointer_cast<IdentifierFrame>(stack.back())->value];
            else index = stack.back();
            stack.pop_back();
            auto arr = stack.back();
            stack.pop_back();
            if(index->getType() != StackFrameType::I64){
                stack.push_back(std::make_shared<FailFrame>()); //error
            } else if(arr->getType() == StackFrameType::IDENTIFIER_FRAME){
                std::string nameOfList = std::static_pointer_cast<IdentifierFrame>(arr)->value;
                std::shared_ptr<ListFrame> list = std::static_pointer_cast<ListFrame>(memory.back()[nameOfList]);
                if(in(nameOfList, memory.back()) && list->getType() == StackFrameType::LIST_FRAME){
                    unsigned i = std::static_pointer_cast<I64Frame>(index)->value;
                    if(i <= list->value.size()){
                        stack.push_back(list->value[i]);
                    } else {
                        stack.push_back(std::make_shared<FailFrame>()); //error
                    }
                } else {
                    stack.push_back(std::make_shared<FailFrame>()); //error
                }
            } else if(arr->getType() == StackFrameType::LIST_FRAME){
                std::shared_ptr<ListFrame> list = std::static_pointer_cast<ListFrame>(arr);
                unsigned i = std::static_pointer_cast<I64Frame>(index)->value;
                if(i <= list->value.size()){
                    stack.push_back(list->value[i]);
                } else {
                    stack.push_back(std::make_shared<FailFrame>()); //error
                }
            } else {
                stack.push_back(std::make_shared<FailFrame>()); //error
            }
            ++ip;
        } else if(currentInstr[0] == "IDE"){
            stack.push_back(std::make_shared<IdentifierFrame>(currentInstr[1]));
            ++ip;
        } else if(currentInstr[0] == "NIL"){
            stack.push_back(std::make_shared<NullFrame>());
            ++ip;
        } else if(currentInstr[0] == "BOOL"){
            stack.push_back(make_shared<BoolFrame>(currentInstr[1] == "TRUE" ? true : false));
            ++ip;
        } else if(currentInstr[0] == "I64"){
            stack.push_back(make_shared<I64Frame>(stoll(currentInstr[1])));
            ++ip;
        } else if(currentInstr[0] == "CHAR"){
            stack.push_back(make_shared<CharFrame>(stod(currentInstr[1])));
            ++ip;
        } else if(currentInstr[0] == "F64"){
            stack.push_back(make_shared<F64Frame>(stod(currentInstr[1])));
            ++ip;
        } else if(currentInstr[0] == "STRING"){
            stack.push_back(make_shared<StringFrame>(currentInstr[1]));
            ++ip;
        } else if(currentInstr[0] == "MAKE_LIST"){
            auto tup = stack.back();
            stack.pop_back();
            if(tup->getType() == StackFrameType::TUPLE_FRAME) stack.push_back(std::make_shared<ListFrame>(std::static_pointer_cast<TupleFrame>(tup)));
            else stack.push_back(std::make_shared<ListFrame>(std::make_shared<TupleFrame>(std::static_pointer_cast<TupleFrame>(tup))));
            ++ip;
        } else if(currentInstr[0] == "EMPTY"){
            stack.push_back(std::make_shared<EmptyFrame>());
            ++ip;
        } else if(currentInstr[0] == "STRUCT"){
            throw "Internal error. Instruction STRUCT should never been executed.";
        } else throw "Internal error. Unknown instruction "+currentInstr[0]+".";
    }
}

void interpreterMain(std::vector<std::vector<std::string>> ir, bool d){
    debug = d;
    auto [ ip, globalMem ] = prepareToExecute(ir);
	try{
        execute(ir, ip, globalMem, std::vector<map<string,shared_ptr<StackFrame>>>(), std::vector<shared_ptr<StackFrame>>(), std::make_shared<NullFrame>(), 0);
    } catch(std::string err) {
        std::cout<< "Interpreter error. " << err <<'\n';
    }
    if(debug){
        cout << "==========================\nExecution completed successfully!\n";
    }
}