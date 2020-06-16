from compiler import parserMain
import concurrent.futures
from stackFrames import *
from copy import deepcopy
from buildinFunctions import buildInDict

def prepareToExecute(ir):
    globalMem = {}
    ip = 0
    while ip < len(ir):
        if ir[ip] and ir[ip][0] == "FUN":
            # print(ir[ip][1])
            if ir[ip][1] in globalMem:
                raise Exception("Reused name "+ir[ip][1]+" in global scope.")
            globalMem[ir[ip][1]] = FunctionLiteral(ip)
        if ir[ip] and ir[ip][0] == "STRUCT":
            # print(ir[ip][1])
            if ir[ip][1] in globalMem:
                raise Exception("Reused name "+ir[ip][1]+" in global scope.")
            globalMem[ir[ip][1]] = StructLiteral(ip)
        ip += 1
    #TODO check if main function exists
    ip = globalMem["main"].line #TODO args passed to program
    return ip, globalMem

def deepc(a):
    b = deepcopy(a)
    return b

def makeNondeterminism(ir, ip, globalMem, memory, stack, deep):
    if not isinstance(stack[-1], YieldBuffer):
        raise Exception("Internal error. YieldBuffer required on a top of the stack.")
    buffer = stack[-1].list
    print(buffer)
    stack.pop()
    all = []
    retip = 0
    # do i need to copy stack?
    # and i think i need only last dictionary in memory
    with concurrent.futures.ThreadPoolExecutor(max_workers=1) as exe:
        results = [exe.submit(execute, ir, ip, globalMem, deepc(memory), deepc(stack), b, deep+1) for b in buffer]
        for res in concurrent.futures.as_completed(results):
            a = res.result()
            retip = a[0] #TODO need some guard
            all.append(a[1])
        while not isinstance(stack[-1], EveryBuffer):
            stack.pop()
        stack.pop()
        stack.append(ListFrame(TupleFrame(all)))
        return retip

def execute(ir, ip, globalMem, memory, stack, onstack, deep):
    if onstack != None:
        stack.append(onstack)
    while True:
        currentInstr = ir[ip]
        # debugging
        print("======== DEBUGGER ========")
        print(globalMem)
        print(memory)
        print(stack)
        print(ip)
        print(deep)
        print("CURRENT INSTRUCTION: ", currentInstr)
        if currentInstr[0] == "SEMICOLON":
            #TODO sprawdzic czy trzeba dorzucić EveryBuffer
            while not (isinstance(stack[-1], FunctionFrame) or isinstance(stack[-1], EveryBuffer)):
                stack.pop()
            ip+=1
        elif currentInstr[0] == "FUN":
            memory.append({})
            if len(stack) > 0:
                args = stack[-1]
                stack.pop()
                if isinstance(args, EmptyFrame):
                    if int(currentInstr[2]) != 0:
                        raise Exception("Function "+currentInstr[1]+" expects "+currentInstr[2]+" arguments. Passed 0.")
                elif len(args.value) != int(currentInstr[2]):
                    raise Exception("Function "+currentInstr[1]+" expects "+currentInstr[2]+" arguments. Passed "+str(len(args.value))+ ".")
                for i in range(0, int(currentInstr[2])):
                    memory[-1][currentInstr[3+i]] = args.value[i]
            stack.append(FunctionFrame(currentInstr[1], ip))
            ip+=1
        elif currentInstr[0] == "yield":
            #TODO jeśli na stosie nie ma everybuffer potraktuj jak ret
            if isinstance(stack[-1], FunctionFrame):
                raise Exception("Internal error. Must yield something")
            toAddToBuffer = stack[-1]
            stack.pop() 
            if not ("?YieldBuffer" in memory[-1]):
                memory[-1]["?YieldBuffer"] = YieldBuffer()
                memory[-1]["?YieldBuffer"].list.append(toAddToBuffer)
            else:
                memory[-1]["?YieldBuffer"].list.append(toAddToBuffer)
            ip+=1
        elif currentInstr[0] == "ENDFUN" or currentInstr[0] == "ret":
            if "?YieldBuffer" in memory[-1]:
                returned = memory[-1]["?YieldBuffer"]
                if len(returned.list) == 1:
                    returned = returned.list[0]
            elif isinstance(stack[-1], FunctionFrame):
                returned = VoidFrame()
            else:
                returned = stack[-1]
            memory.pop()
            while not isinstance(stack[-1], FunctionFrame):
                stack.pop()
            if isinstance(stack[-1], FunctionFrame):
                if len(stack) == 1 and stack[-1].name == "main":
                    break #HALT
                ip = stack[-2].line+1
                del stack[-2:]
                stack.append(returned)
                if isinstance(stack[-1], YieldBuffer):
                    ip = makeNondeterminism(ir, ip, globalMem, memory, stack, deep)
                    if(deep>0):
                        return ip, (memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1])
            else:
                raise Exception("Internal error.")
        elif currentInstr[0] == "STRUCT":
            raise Exception("Internal error. Instruction STRUCT should never been executed.")
        elif currentInstr[0] == "=":
            #TODO
            assignFrom = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            assignTo = stack[-2]
            memory[-1][assignTo.value] = assignFrom
            del stack[-2:]
            stack.append(assignTo)
            ip+=1
        elif currentInstr[0] == "EVERY":
            stack.append(EveryBuffer())
            ip+=1
        elif currentInstr[0] == "EVERYBODY" or currentInstr[0] == "and":
            if isinstance(stack[-1], FailFrame):
                ip = currentInstr[2]
            while not isinstance(stack[-1], EveryBuffer):
                stack.pop()
            ip+=1 #TODO ip zmienia się 2 razy - czy to ok?
        elif currentInstr[0] == "ENDEVERY":
            return ip+1, (memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1])
        elif currentInstr[0] == "DOWHILEBODY":
            ip+=1
        elif currentInstr[0] == "DOWHILE":
            ip+=1
        elif currentInstr[0] == "ENDDOWHILE":
            #TODO condition must be Bool
            condition = stack[-1]
            stack.pop()
            if condition:
                ip = currentInstr[2]
            else:
                ip += 1
        elif currentInstr[0] == "while":
            ip+=1
        elif currentInstr[0] == "DO" or currentInstr[0] == "THEN":
            condition = stack[-1]
            stack.pop()
            if condition:
                ip += 1
            else:
                ip = currentInstr[2]
        elif currentInstr[0] == "ENDWHILE":
            ip = currentInstr[2]
        elif currentInstr[0] == "IF":
            ip+=1
        elif currentInstr[0] == "ELSE":
            ip = currentInstr[2]
        elif currentInstr[0] == "ENDIF":
            ip+=1
        elif currentInstr[0] == "break":
            ip = currentInstr[2] #TODO posprzątać stos
        elif currentInstr[0] == "continue":
            ip = currentInstr[2] #TODO posprzątać stos
        elif currentInstr[0] == "COMMA":
            nrOfElems = -int(currentInstr[1])
            #TODO usunąć indeksy
            result = TupleFrame(stack[nrOfElems:])
            del stack[nrOfElems:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "OR":
            nrOfElems = -int(currentInstr[1])
            yb = YieldBuffer()
            yb.list += stack[nrOfElems:]
            del stack[nrOfElems:]
            print(yb)
            stack.append(yb)
            ip = makeNondeterminism(ir, ip+1, globalMem, memory, stack, deep)
            if(deep>0):
                return ip, (memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1])
        elif currentInstr[0] == "NOT":
            if isinstance(stack[-1], FailFrame):
                stack.pop()
            ip+=1
        elif currentInstr[0] == "&&":
            #TODO error if not Bool
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b and a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "||":
            #TODO error if not Bool
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b or a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "|":
            #TODO error if not Bool
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b | a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "^":
            #TODO error if not Bool
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b ^ a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "&":
            #TODO error if not Bool
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b & a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "==":
            #TODO error if not I64 or F64 or Char or Bool or String
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b == a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "!=":
            #TODO error if not I64 or F64 or Char or Bool or String
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b != a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == ">=":
            #TODO error if not I64 or F64
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b >= a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "<=":
            #TODO error if not I64 or F64
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b <= a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "<":
            #TODO error if not I64 or F64
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b < a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == ">":
            #TODO error if not I64 or F64
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b > a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == ">>":
            #TODO error if not I64
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b >> a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "<<":
            #TODO error if not I64
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b << a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "-":
            #TODO error if not I64 or F64
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = b - a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "+":
            #TODO error if not I64 or F64 or String
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            result = a + b
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "*":
            #TODO error if not I64 or F64
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            if b == 0:
                stack.append(FailFrame())
            result = b * a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "/":
            #TODO error if not I64 or F64
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            if b == 0:
                stack.append(FailFrame())
            result = b / a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "%":
            #TODO error if not I64 or F64
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            b = memory[-1][stack[-2].value] if isinstance(stack[-2], IdentifierFrame) else stack[-2]
            if b == 0:
                stack.append(FailFrame())
            result = b % a
            del stack[-2:]
            stack.append(result)
            ip+=1
        elif currentInstr[0] == "~":
            #TODO error if not Bool
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            stack.pop()
            stack.append(not a)
            ip+=1
        elif currentInstr[0] == "MINUS":
            #TODO error if not I64 or F64
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            stack.pop()
            stack.append(-a)
            ip+=1
        elif currentInstr[0] == "!":
            #TODO error if not Bool
            a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            stack.pop()
            stack.append(not a)
            ip+=1
        elif currentInstr[0] == "(":
            callingFunction = stack[-2]
            if callingFunction.value in globalMem:
                nextIp = globalMem[callingFunction.value].line
                args = stack[-1]
                del stack[-2:]
                stack.append(RetFromFunctionFrame(ip))
                ip = nextIp
                stack.append(args)
            elif callingFunction.value in buildInDict:
                args = stack[-1]
                for a in args:
                    a = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else a
                del stack[-2:]
                stack.append(buildInDict[callingFunction.value](args))
                ip += 1
            else: #Trying to call undefined function
                stack.append(FailFrame())
        elif currentInstr[0] == ".":
            objectName = stack[-2].value
            fieldName = stack[-2].value
            del stack[-2:]
            stack.append(memory[-1][objectName].struct[fieldName])
            ip+=1
        elif currentInstr[0] == "{":
            structName = stack[-2].value
            tup = stack[-1]
            if not (structName in globalMem):
                raise Exception("Struct "+structName+" not implemented.")
            if len(tup.value) != int(ir[globalMem[structName].line][2]):
                raise Exception("Struct "+structName+" contains "+str(len(tup.value))+" elements. ")
            dic = {}
            i = 0
            while i < len(tup.value):
                dic[ir[globalMem[structName].line][i+3]] = tup.value[i]
                i += 1
            del stack[-2:]
            stack.append(ObjectFrame(structName, dic))
            ip+=1
        elif currentInstr[0] == "[":
            nameOfList = stack[-2].value
            index = memory[-1][stack[-1].value] if isinstance(stack[-1], IdentifierFrame) else stack[-1]
            del stack[-2:]
            if not isinstance(index, int):
                raise Exception("List index must be an integer. "+str(type(index))+" given.")
            elif not (nameOfList in memory[-1]):
                raise Exception(nameOfList+ " - no such identifier.")
            elif not isinstance(memory[-1][nameOfList], ListFrame):
                raise Exception(nameOfList+ "is not a list.")
            elif index >= len(memory[-1][nameOfList].value):
                raise Exception("Index "+str(index)+" out of range. List "+nameOfList+" has only "+str(len(memory[-1][nameOfList].value))+" elements.")
            else:
                stack.append(memory[-1][nameOfList].value[index])
            ip+=1
        elif currentInstr[0] == "IDE":
            stack.append(IdentifierFrame(currentInstr[1]))
            ip+=1
        elif currentInstr[0] == "NIL":
            stack.append(None)
            ip+=1
        elif currentInstr[0] == "BOOL":
            stack.append(True if currentInstr[1] == "True" else False) 
            ip+=1
        elif currentInstr[0] == "I64":
            stack.append(int(currentInstr[1]))
            ip+=1
        elif currentInstr[0] == "CHAR":
            raise Exception("Not implemented yet.")
            ip+=1 #TODO
        elif currentInstr[0] == "F64":
            raise Exception("Not implemented yet.")
            ip+=1 #TODO
        elif currentInstr[0] == "STRING":
            raise Exception("Not implemented yet.")
            ip+=1 #TODO
        elif currentInstr[0] == "MAKE_LIST":
            tup = stack[-1]
            stack.pop()
            stack.append(ListFrame(tup))
            ip+=1
        elif currentInstr[0] == "EMPTY":
            stack.append(EmptyFrame())
            ip+=1
        else:
            raise Exception("Internal error. Unknown instruction.")
        # input("")
        
def main():
    with open('ex.txt', 'r') as file:
        data = file.read()
    ir = parserMain(data) #ir:[[]] - list of instructions
    print("======== CODE ========")
    i = 0
    for a in ir:
        print(i, a)
        i += 1

    # try:
    ip, globalMem = prepareToExecute(ir)
    execute(ir, ip, globalMem, [], [], None, 0)
    # except Exception as e:
    #     print("Error.", e)

main()