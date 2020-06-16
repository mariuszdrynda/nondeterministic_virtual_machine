#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <cmath>
#include <mutex>

enum class StackFrameType{
    FUNCTION_LITERAL, STRUCT_LITERAL, NULL_FRAME, FUNCTION_FRAME, EVERY_BUFFER, 
    VOID_FRAME, IDENTIFIER_FRAME, TUPLE_FRAME, RET_FROM_FUNCTION_FRAME, 
    LIST_FRAME, YIELD_BUFFER, REFFERENCE_ARRAY_ELEM, REFFERENCE_FIELD, EMPTY_FRAME,
    FAIL_FRAME, OBJECT_FRAME, STRING, CHAR, BOOL, I64, F64, ERROR
};
struct StackFrame{
    virtual StackFrameType getType(){
        throw "Internal error. Trying to access unspecified Stack Frame.";
    }
    virtual unsigned getLine(){
        throw "Internal error. Trying to access unspecified Stack Frame.";
    }
    virtual std::string print() const{
        throw "Internal error. Trying to access unspecified Stack Frame.";
    }
    virtual std::string getValue(){
        throw "Internal error. Trying to access unspecified Stack Frame.";
    }
    virtual std::string cout(){
        throw "Internal error. Trying to access unspecified Stack Frame.";
    }
    virtual std::shared_ptr<StackFrame> clone(){
        throw "Internal error. Trying to access unspecified Stack Frame.";
    }
};
struct FunctionFrame : StackFrame{
    FunctionFrame(std::string n, unsigned cl): name(n), callingLine(cl){}
    StackFrameType getType() override{
        return StackFrameType::FUNCTION_FRAME;
    }
    unsigned getLine() override{
        return 0;
    }
    std::string getValue(){
        return name;
    }
    std::string print() const override{
        return "<FN FRAME "+name+" "+std::to_string(callingLine)+">";
    }
    std::string cout() override{
        throw "cout function not implemented yet";
        return "";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<FunctionFrame>(name, callingLine);
    }
    std::string name;
    unsigned callingLine;
};
struct FunctionLiteral : StackFrame{
    FunctionLiteral(unsigned l): line(l){}
    StackFrameType getType() override{
        return StackFrameType::FUNCTION_LITERAL;
    }
    unsigned getLine() override{
        return line;
    }
    std::string print() const override{
        return "<FUNCTION LITERAL "+std::to_string(line)+">";
    }
    std::string cout() override{
        throw "cout function not implemented yet";
        return "";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<FunctionLiteral>(line);
    }
    unsigned line;
};
struct StructLiteral : StackFrame{
    StructLiteral(unsigned l): line(l){}
    StackFrameType getType() override{
        return StackFrameType::STRUCT_LITERAL;
    }
    std::string print() const override{
        return "<STRUCT LITERAL "+std::to_string(line)+">";
    }
    std::string cout() override{
        throw "cout function not implemented yet";
        return "";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<StructLiteral>(line);
    }
    unsigned line;
};
struct NullFrame : StackFrame{
    StackFrameType getType() override{
        return StackFrameType::NULL_FRAME;
    }
    std::string print() const override{
        return "";
    }
    std::string cout() override{
        return "nil";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<NullFrame>();
    }
};
struct IdentifierFrame : StackFrame{
    IdentifierFrame(std::string v): value(v){}
    StackFrameType getType() override{
        return StackFrameType::IDENTIFIER_FRAME;
    }
    std::string print() const override{
        return "<IDE FRAME "+value+">";
    }
    std::string getValue() override{
        return value;
    }
    std::string cout() override{
        throw "cout function not implemented yet";
        return "";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<IdentifierFrame>(value);
    }
    std::string value;
};
struct TupleFrame : StackFrame{
    TupleFrame(std::vector<std::shared_ptr<StackFrame>> v): value(v){}
    TupleFrame(std::shared_ptr<StackFrame> v){
        value.push_back(v);
    }
    StackFrameType getType() override{
        return StackFrameType::TUPLE_FRAME;
    }
    std::string print() const override{
        std::string result = "<TUPLE ";
        for(auto& a : value) result += a->print();
        result += ">";
        return result;
    }
    std::string cout() override{
        std::string result = "";
        for(auto& a : value) result += a->cout();
        return result;
    }
    std::shared_ptr<StackFrame> clone(){
        std::vector<std::shared_ptr<StackFrame>> ret;
        ret.reserve(value.size());
        for(auto& i : value){
            ret.push_back(i->clone());
        }
        return std::make_shared<TupleFrame>(ret);
    }
    std::vector<std::shared_ptr<StackFrame>> value;
};
struct VoidFrame : StackFrame{
    VoidFrame(){}
    StackFrameType getType() override{
        return StackFrameType::VOID_FRAME;
    }
    std::string print() const override{
        return "<VOID FRAME>";
    }
    std::string cout() override{
        return "";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<VoidFrame>();
    }
};
struct RetFromFunctionFrame : StackFrame{
    RetFromFunctionFrame(unsigned l): line(l){}
    StackFrameType getType() override{
        return StackFrameType::RET_FROM_FUNCTION_FRAME;
    }
    std::string print() const override{
        return "<RET FROM FN FRAME "+std::to_string(line)+">";
    }
    std::string cout() override{
        throw "cout function not implemented yet";
        return "";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<RetFromFunctionFrame>(line);
    }
    unsigned line;
};
struct EmptyFrame : StackFrame{
    EmptyFrame(){}
    StackFrameType getType() override{
        return StackFrameType::EMPTY_FRAME;
    }
    std::string print() const override{
        return "<EMPTY FRAME>";
    }
    std::string cout() override{
        return "";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<EmptyFrame>();
    }
};
struct ListFrame : StackFrame{
    ListFrame(std::shared_ptr<TupleFrame> inputTuple){
        value = inputTuple->value;
    }
    StackFrameType getType() override{
        return StackFrameType::LIST_FRAME;
    }
    std::string print() const override{
        std::string result = "<LIST FRAME ";
        for(auto& a : value) result += a->print();
        result += ">";
        return result;
    }
    std::string cout() override{
        std::string result = "";
        for(auto& a : value) result += a->cout() + ", ";
        return result;
    }
    void update(std::shared_ptr<StackFrame> val, unsigned index){
        value[index] = val;
    }
    std::shared_ptr<StackFrame> clone(){
        std::vector<std::shared_ptr<StackFrame>> ret;
        ret.reserve(value.size());
        for(auto& i : value){
            ret.push_back(i->clone());
        }
        return std::make_shared<ListFrame>(std::make_shared<TupleFrame>(ret));
    }
    std::vector<std::shared_ptr<StackFrame>> value;
};
struct EveryBuffer : StackFrame{
    EveryBuffer(){}
    StackFrameType getType() override{
        return StackFrameType::EVERY_BUFFER;
    }
    std::string print() const override{
        return "<EVERY BUFFER>";
    }
    std::string cout() override{
        throw "cout function not implemented yet";
        return "";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<EveryBuffer>();
    }
};
struct FailFrame : StackFrame{
    FailFrame(){}
    StackFrameType getType() override{
        return StackFrameType::FAIL_FRAME;
    }
    std::string print() const override{
        return "<FAIL FRAME>";
    }
    std::string cout() override{
        return "<FAIL FRAME>";
        return "";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<FailFrame>();
    }
};
struct YieldBuffer : StackFrame{
    YieldBuffer(std::shared_ptr<StackFrame> input){
        list.push_back(input);
    }
    StackFrameType getType() override{
        return StackFrameType::YIELD_BUFFER;
    }
    std::string print() const override{
        std::string result = "<YIELD BUFFER ";
        for(auto& a : list) result += a->print();
        result += ">";
        return result;
    }
    std::string cout() override{
        throw "cout function not implemented yet";
        return "";
    }
    std::shared_ptr<StackFrame> clone(){
        std::shared_ptr<YieldBuffer> y;
        std::vector<std::shared_ptr<StackFrame>> ret;
        ret.reserve(list.size());
        for(auto& i : list){
            ret.push_back(i->clone());
        }
        y->list = ret;
        return y;
    }
    std::vector<std::shared_ptr<StackFrame>> list;
};
struct BoolFrame : StackFrame{
    BoolFrame(bool i): value(i){}
    StackFrameType getType() override{
        return StackFrameType::BOOL;
    }
    std::string print() const override{
        return "<BOOL "+std::to_string(value)+">";
    }
    std::string cout() override{
        return value ? "1" : "0";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<BoolFrame>(value);
    }
    bool value;
};
struct ObjectFrame : StackFrame{
    ObjectFrame(std::string structName, std::map<std::string, std::shared_ptr<StackFrame>> inputDictionary)
        : type(structName), str(inputDictionary){}
    StackFrameType getType() override{
        return StackFrameType::OBJECT_FRAME;
    }
    std::string print() const override{
        std::string result = "<OBJECT FRAME "+type+" {";
        for(auto it = str.cbegin(); it != str.cend(); ++it){
            result += it->first + " " + it->second->print() + ", ";
        }
        result += "} >";
        return result;
    }
    std::string cout() override{
        std::string result = type+" {";
        for(auto it = str.cbegin(); it != str.cend(); ++it){
            result += it->first + " " + it->second->cout() + ", ";
        }
        result += "}";
        return result;
    }
    void update(std::shared_ptr<StackFrame> val, std::string field){
        str[field] = val;
    }
    bool in(std::string searched){
        auto it = str.find(searched);
        if (it != str.end()) return true;
        else return false;
    }
    std::shared_ptr<StackFrame> clone(){
        std::map<std::string, std::shared_ptr<StackFrame>> ret;
        std::map<std::string, std::shared_ptr<StackFrame>>::iterator it = str.begin();
        while (it != str.end()){
            std::string word = it->first;
            std::shared_ptr<StackFrame> copied= it->second->clone();
            ret[word] = copied;
            it++;
        }
        return std::make_shared<ObjectFrame>(type, ret);
    }
    std::string type;
    std::map<std::string, std::shared_ptr<StackFrame>> str;
};
struct I64Frame : StackFrame{
    I64Frame(long long v): value(v){}
    std::string print() const override{
        return "<INTEGER LITERAL "+std::to_string(value)+">";
    }
    std::string getValue() override{
        return std::to_string(value);
    }
    StackFrameType getType() override{
        return StackFrameType::I64;
    }
    std::string cout() override{
        return std::to_string(value);
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<I64Frame>(value);
    }
    long long value;
};
struct F64Frame : StackFrame{
    F64Frame(double v): value(v){}
    std::string print() const override{
        return "<FLOATING LITERAL "+std::to_string(value)+">";
    }
    std::string getValue() override{
        return std::to_string(value);
    }
    StackFrameType getType() override{
        return StackFrameType::F64;
    }
    std::string cout() override{
        return std::to_string(value);
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<F64Frame>(value);
    }
    double value;
};
struct StringFrame : StackFrame{
    StringFrame(std::string v): value(v){}
    std::string print() const override{
        return "<STRING LITERAL "+value+">";
    }
    std::string getValue() override{
        return value;
    }
    StackFrameType getType() override{
        return StackFrameType::STRING;
    }
    std::string cout() override{
        return value;
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<StringFrame>(value);
    }
    std::string value;
};
struct CharFrame : StackFrame{
    CharFrame(char v): value(v){}
    std::string print() const override{
        return "<CHAR LITERAL "+std::string(1, value)+">";
    }
    std::string getValue() override{
        return std::string(1, value);
    }
    StackFrameType getType() override{
        return StackFrameType::CHAR;
    }
    std::string cout() override{
        return std::string(1, value);
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<CharFrame>(value);
    }
    char value;
};
struct RefferenceArrayElem : StackFrame{
    RefferenceArrayElem(std::shared_ptr<StackFrame> l, unsigned i): left(l), index(i){}
    std::string print() const override{
        return "<REFFERENCE ARRAY ELEM "+left->print()+" "+std::to_string(index)+">";
    }
    std::string getValue() override{
        return "";
    }
    StackFrameType getType() override{
        return StackFrameType::REFFERENCE_ARRAY_ELEM;
    }
    std::string cout() override{
        return "";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<RefferenceArrayElem>(left->clone(), index);
    }
    std::shared_ptr<StackFrame> left;
    unsigned index;
};
struct RefferenceField: StackFrame{
    RefferenceField(std::shared_ptr<StackFrame> l, std::string f): left(l), fieldName(f){}
    std::string print() const override{
        return "<REFFERENCE FIELD "+left->print()+" "+fieldName+">";
    }
    std::string getValue() override{
        return "";
    }
    StackFrameType getType() override{
        return StackFrameType::REFFERENCE_FIELD;
    }
    std::string cout() override{
        return "";
    }
    std::shared_ptr<StackFrame> clone(){
        return std::make_shared<RefferenceField>(left->clone(), fieldName);
    }
    std::shared_ptr<StackFrame> left;
    std::string fieldName;
};

// =============================================== BUILD-IN ===============================================

std::mutex io;

std::shared_ptr<StackFrame> getln_bi(std::shared_ptr<StackFrame> args){
    std::string input;
    io.lock();
    std::cin >> input;
    io.unlock();
    return std::make_shared<StringFrame>(input);
}

std::shared_ptr<StackFrame> println_bi(std::shared_ptr<StackFrame> args){
    io.lock();
    if(args->getType() == StackFrameType::I64 || 
        args->getType() == StackFrameType::F64 ||
        args->getType() == StackFrameType::BOOL ||
        args->getType() == StackFrameType::CHAR ||
        args->getType() == StackFrameType::STRING)
            std::cout<<args->getValue()<<'\n';
    else if(args->getType() == StackFrameType::TUPLE_FRAME){
        auto tuple = std::static_pointer_cast<TupleFrame>(args);
        for(auto& i : tuple->value){
            std::cout<<i->cout()<<" ";
        }
        std::cout<<'\n';
    } else if(args->getType() == StackFrameType::LIST_FRAME){
        auto list = std::static_pointer_cast<ListFrame>(args);
        for(auto& i : list->value){
            std::cout<<i->cout()<<" ";
        }
        std::cout<<'\n';
    }
    io.unlock();
    return std::make_shared<VoidFrame>();
}
std::shared_ptr<StackFrame> int_bi(std::shared_ptr<StackFrame> arg){
    if(arg->getType() == StackFrameType::TUPLE_FRAME){
        auto args = std::static_pointer_cast<TupleFrame>(arg)->value[0];
        arg = args;
    }
    if(arg->getType() == StackFrameType::I64)
        return arg;
    else if(arg->getType() == StackFrameType::F64)
        return std::make_shared<I64Frame>(std::static_pointer_cast<F64Frame>(arg)->value);
    else if(arg->getType() == StackFrameType::BOOL)
        return std::make_shared<I64Frame>(std::static_pointer_cast<I64Frame>(arg)->value);
    else if(arg->getType() == StackFrameType::CHAR)
        return std::make_shared<I64Frame>(std::static_pointer_cast<CharFrame>(arg)->value);
    else if(arg->getType() == StackFrameType::STRING){
        return std::make_shared<I64Frame>(stoll(std::static_pointer_cast<StringFrame>(arg)->value));
    } else {
        std::make_shared<FailFrame>();
    }
}
std::shared_ptr<StackFrame> sqrt_bi(std::shared_ptr<StackFrame> args){
    if(args->getType() == StackFrameType::TUPLE_FRAME){
        auto arg = std::static_pointer_cast<TupleFrame>(args)->value[0];
        if(arg->getType() == StackFrameType::I64)
            return std::make_shared<F64Frame>(std::sqrt(std::static_pointer_cast<I64Frame>(arg)->value));
        else if(arg->getType() == StackFrameType::F64){
            return std::make_shared<F64Frame>(std::sqrt(std::static_pointer_cast<F64Frame>(arg)->value));
        } else
            std::make_shared<FailFrame>();
    } else {
        std::make_shared<FailFrame>();
    }
}

std::shared_ptr<StackFrame> float_bi(std::shared_ptr<StackFrame> arg){
    if(arg->getType() == StackFrameType::TUPLE_FRAME){
        auto args = std::static_pointer_cast<TupleFrame>(arg)->value[0];
        arg = args;
    }
    if(arg->getType() == StackFrameType::I64)
        return std::make_shared<F64Frame>(std::static_pointer_cast<F64Frame>(arg)->value);
    else if(arg->getType() == StackFrameType::F64)
        return arg;
    else if(arg->getType() == StackFrameType::BOOL)
        return std::make_shared<F64Frame>(std::static_pointer_cast<I64Frame>(arg)->value);
    else if(arg->getType() == StackFrameType::CHAR)
        return std::make_shared<F64Frame>(std::static_pointer_cast<CharFrame>(arg)->value);
    else if(arg->getType() == StackFrameType::STRING){
        return std::make_shared<F64Frame>(stod(std::static_pointer_cast<StringFrame>(arg)->value));
    } else {
        std::make_shared<FailFrame>();
    }
}
std::shared_ptr<StackFrame> len_bi(std::shared_ptr<StackFrame> arg){
    if(arg->getType() == StackFrameType::TUPLE_FRAME){
        auto args = std::static_pointer_cast<TupleFrame>(arg)->value[0];
        arg = args;
    }
    if(arg->getType() == StackFrameType::LIST_FRAME){
        unsigned s = std::static_pointer_cast<ListFrame>(arg)->value.size();
        return std::make_shared<I64Frame>(s);
    } else {
        std::make_shared<FailFrame>();
    }
}
std::shared_ptr<StackFrame> instance_bi(std::shared_ptr<StackFrame> arg){
    if(arg->getType() == StackFrameType::OBJECT_FRAME){
        auto s = std::static_pointer_cast<ObjectFrame>(arg)->type;
        return std::make_shared<StringFrame>(s);
    } else {
        std::make_shared<FailFrame>();
    }
}
std::shared_ptr<StackFrame> append_bi(std::shared_ptr<StackFrame> arg){
    if(arg->getType() == StackFrameType::TUPLE_FRAME){
        auto lista = std::static_pointer_cast<TupleFrame>(arg)->value[0];
        auto var = std::static_pointer_cast<TupleFrame>(arg)->value[1];
        if(lista->getType() == StackFrameType::LIST_FRAME){
            auto l = std::static_pointer_cast<ListFrame>(lista)->clone();
            auto ll = std::static_pointer_cast<ListFrame>(l);
            ll->value.push_back(var);
            return ll;
        } else {
            std::make_shared<FailFrame>();
        }
    } else {
        std::make_shared<FailFrame>();
    }
}
std::shared_ptr<StackFrame> pop_bi(std::shared_ptr<StackFrame> arg){
    if(arg->getType() == StackFrameType::TUPLE_FRAME){
        auto lista = std::static_pointer_cast<TupleFrame>(arg)->value[0];
        if(lista->getType() == StackFrameType::LIST_FRAME){
            auto l = std::static_pointer_cast<ListFrame>(lista)->clone();
            auto ll = std::static_pointer_cast<ListFrame>(l);
            ll->value.pop_back();
            return ll;
        } else {
            std::make_shared<FailFrame>();
        }
    } else {
        std::make_shared<FailFrame>();
    }
}
std::shared_ptr<StackFrame> back_bi(std::shared_ptr<StackFrame> arg){
    if(arg->getType() == StackFrameType::TUPLE_FRAME){
        auto lista = std::static_pointer_cast<TupleFrame>(arg)->value[0];
        if(lista->getType() == StackFrameType::LIST_FRAME){
            auto l = std::static_pointer_cast<ListFrame>(lista);
            auto ll = l->value.back()->clone();
            return ll;
        } else {
            std::make_shared<FailFrame>();
        }
    } else {
        std::make_shared<FailFrame>();
    }
}

std::map<std::string, std::function<std::shared_ptr<StackFrame>(std::shared_ptr<StackFrame>)>> buildInDict
    = {{"println", println_bi}, {"int", int_bi}, {"float", float_bi}, {"sqrt", sqrt_bi}, {"len", len_bi},
    {"instance", instance_bi}, {"append", append_bi}, {"pop", pop_bi}, {"back", back_bi}, {"getln", getln_bi}};