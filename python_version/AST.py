from enum import Enum

loopNr = 0
ifNr = 0
statementNr = 0

class LiteralType(Enum):
    ID = "IDE"
    NIL = "NIL"
    BOOL = "BOOL"
    I64_LITERAL = "I64"
    CHAR_LITERAL = "CHAR"
    F64_LITERAL = "F64"
    STRING_LITERAL = "STRING"
    EMPTY = "EMPTY"

class Literal:
    def __init__(self, literalType, value):
        self.literalType = literalType
        self.value = value
    def __repr__(self):
        if self.value != None:
            return self.literalType.value+" "+self.value.__repr__()+"\n"
        else:
            return self.literalType.value+"\n"

class Identifier:
    def __init__(self, value):
        self.value = value
    def __repr__(self):
        return "IDE "+self.value+"\n"

class Function:
    def __init__(self, name, argList, body):
        self.name = name
        self.argList = argList
        self.body = body
    def __repr__(self):
        return "FUN "+self.name+" "+self.argList.__repr__()+"\n"+self.body.__repr__()+"ENDFUN\n"
        # return ["FUN", self.name, self.argList.__repr__(), self.body.__repr__(), "ENDFUNC"]

class Struct:
    def __init__(self, name, body):
        self.name = name
        self.body = body
    def __repr__(self):
        return "STRUCT "+self.name+" "+self.body.__repr__()+"\n"

class Field:
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return "{Field, name: "+self.name+"}"

class Return:
    def __init__(self, returnType, expression):
        self.returnType = returnType
        self.expression = expression
    def __repr__(self):
        return self.expression.__repr__()+self.returnType+"\n"

# class Switch:
#     def __init__(self, statementType, condition, body):
#         self.statementType = statementType
#         self.condition = condition
#         self.body = body
#     def __repr__(self):
#         global statementNr
#         statementNr += 1
#         curr = statementNr
#         result = "SWITCH "+str(curr)+"\n"+self.condition.__repr__()+self.body.__repr__()+"ENDSWITCH "+str(curr)+"\n"
#         statementNr -= 1
#         return result

class Statement:
    def __init__(self, statementType, condition, body):
        self.statementType = statementType
        self.condition = condition
        self.body = body
    def __repr__(self):
        global loopNr
        curr = loopNr
        loopNr += 1
        if self.statementType == "while":
            return self.statementType+" "+str(curr)+"\n"+self.condition.__repr__()+"DO "+str(curr)+"\n"+self.body.__repr__()+"ENDWHILE "+str(curr)+"\n"
        elif self.statementType == "do":
            return "DOWHILE "+str(curr)+"\n"+self.body.__repr__()+self.statementType+" "+str(curr)+"\n"+self.condition.__repr__()+"ENDDOWHILE "+str(curr)+"\n"
        elif self.statementType == "every":
            return "EVERY "+str(curr)+"\n"+self.condition.__repr__()+"EVERYBODY "+str(curr)+"\n"+self.body.__repr__()+"ENDEVERY "+str(curr)+"\n"
        # elif self.statementType == "case":
        #     global statementNr
        #     curr = statementNr
        #     result = "CASE "+str(curr)+"\n"+self.condition.__repr__()+"ARROW "+str(curr)+"\n"+self.body.__repr__()+"ENDCASE\n"
        #     return result
        else:
            raise Exception("Internal error. Wrong statement type.")

class ifStatement:
    def __init__(self, condition, thenBody, elseBody):
        self.condition = condition
        self.thenBody = thenBody
        self.elseBody = elseBody
    def __repr__(self):
        global ifNr
        curr = ifNr
        ifNr += 1
        return "IF "+str(curr)+"\n"+self.condition.__repr__()+"THEN "+str(curr)+"\n"+self.thenBody.__repr__()+"ELSE "+str(curr)+"\n"+(self.elseBody.__repr__() if self.elseBody != None else "")+"ENDIF "+str(curr)+"\n"

class Breaker:
    #TODO dopuszczalne tylko w dowhile / while / switch
    def __init__(self, breakerType):
        self.breakerType = breakerType
    def __repr__(self):
        global loopNr
        return self.breakerType+" "+str(loopNr-1)+"\n"

class Expression:
    def __init__(self, left, right, operator):
        self.left = left
        self.right = right
        self.operator = operator
    def __repr__(self):
        #TODO semanticAnalyzer: czy po lewej stronie jest poprawne wyraenie
        return self.left.__repr__()+self.right.__repr__()+self.operator+"\n"

class UnaryExpression:
    def __init__(self, exp, operator):
        self.expression = exp
        self.operator = operator
    def __repr__(self):
        return self.expression.__repr__()+("MINUS" if self.operator == '-' else self.operator)+"\n"

class ListType(Enum):
    COMMA = "COMMA"
    SEMICOLON = "SEMICOLON"
    ARGS = "ARGS"
    FIELDS = "FIELDS"
    CASELIST = "CASELIST"
    OR = "OR"

class List:
    def __init__(self, element, t):
        self.l = []
        self.type = t
        if element != None:
            self.l.append(element)
    def append(self, elem):
        self.l.append(elem)
    def __repr__(self):
        if self.type == ListType.SEMICOLON:
            ir = ""
            for a in self.l:
                ir += a.__repr__()+self.type.value+"\n"
            return ir
        elif self.type == ListType.ARGS or self.type == ListType.FIELDS:
            ir = ""
            for a in self.l:
                ir += a + " "
            return str(len(self.l))+" "+ir
        elif self.type == ListType.CASELIST:
            ir = ""
            for a in self.l:
                ir += a.__repr__()
            return ir
        else:
            ir = ""
            for a in self.l:
                ir += a.__repr__()
            return ir+self.type.value+" "+str(len(self.l))+"\n"