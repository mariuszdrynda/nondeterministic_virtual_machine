from enum import Enum

loopNr = 0
ifNr = 0
statementNr = 0

class SemanticChecker:
    def __init__(self, assign):
        self.assign = assign #1 - left, -1 - right, 0 - none

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
    def semantic(self, sc):
        if sc.assign == 1:
            raise Exception("Literal on the left side of assignment expression")
    def __repr__(self):
        if self.value != None:
            return self.literalType.value+" "+self.value.__repr__()+"\n"
        else:
            return self.literalType.value+"\n"

class Identifier:
    def __init__(self, value):
        self.value = value
    def semantic(self, sc):
        return #ok
    def __repr__(self):
        return "IDE "+self.value+"\n"

class Function:
    def __init__(self, name, argList, body):
        self.name = name
        self.argList = argList
        self.body = body
    def semantic(self, sc):
        self.body.semantic(sc)
    def __repr__(self):
        return "FUN "+self.name+" "+self.argList.__repr__()+"\n"+self.body.__repr__()+"ENDFUN\n"

class Struct:
    def __init__(self, name, body):
        self.name = name
        self.body = body
    def semantic(self, sc):
        return #ok
    def __repr__(self):
        return "STRUCT "+self.name+" "+self.body.__repr__()+"\n"

class Return:
    def __init__(self, returnType, expression):
        self.returnType = returnType
        self.expression = expression
    def semantic(self, sc):
        if sc.assign == 1:
            raise Exception(self.returnType+" on the left side of assignment expression")
        else:
            self.expression.semantic(sc)
    def __repr__(self):
        return self.expression.__repr__()+self.returnType+"\n"

class Statement:
    def __init__(self, statementType, condition, body):
        self.statementType = statementType
        self.condition = condition
        self.body = body
    def semantic(self, sc):
        if sc.assign == 1:
            raise Exception(self.statementType+" on the left side of assignment expression")
        else:
            self.condition.semantic(SemanticChecker(sc))
            self.body.semantic(SemanticChecker(sc))
    def __repr__(self):
        global loopNr
        curr = loopNr
        loopNr += 1
        if self.statementType == "while":
            return "while "+str(curr)+"\n"+self.condition.__repr__()+"DO "+str(curr)+"\n"+self.body.__repr__()+"ENDWHILE "+str(curr)+"\n"
        elif self.statementType == "do":
            return "DOWHILE "+str(curr)+"\n"+self.body.__repr__()+"DOWHILEBODY "+str(curr)+"\n"+self.condition.__repr__()+"ENDDOWHILE "+str(curr)+"\n"
        elif self.statementType == "every":
            return "EVERY "+str(curr)+"\n"+self.condition.__repr__()+"EVERYBODY "+str(curr)+"\n"+self.body.__repr__()+"ENDEVERY "+str(curr)+"\n"
        else:
            raise Exception("Internal error. Wrong statement type.")

class ifStatement:
    def __init__(self, condition, thenBody, elseBody):
        self.condition = condition
        self.thenBody = thenBody
        self.elseBody = elseBody
    def semantic(self, sc):
        if sc.assign == 1:
            raise Exception("if statement on the left side of assignment expression")
        else:
            self.condition.semantic(SemanticChecker(0))
            self.thenBody.semantic(SemanticChecker(0))
            if self.elseBody != None: self.elseBody.semantic(SemanticChecker(0))
    def __repr__(self):
        global ifNr
        curr = ifNr
        ifNr += 1
        return "IF "+str(curr)+"\n"+self.condition.__repr__()+"THEN "+str(curr)+"\n"+self.thenBody.__repr__()+"ELSE "+str(curr)+"\n"+(self.elseBody.__repr__() if self.elseBody != None else "")+"ENDIF "+str(curr)+"\n"

class Breaker:
    def __init__(self, breakerType):
        self.breakerType = breakerType
    def semantic(self, sc):
        if sc.assign == 1 or sc.assign == -1:
            raise Exception(str(self.breakerType)+" inside assignment expression")
    def __repr__(self):
        global loopNr
        return self.breakerType+" "+str(loopNr-1)+"\n"

class Expression:
    def __init__(self, left, right, operator):
        self.left = left
        self.right = right
        self.operator = operator
    def semantic(self, sc):
        if sc.assign == 1:
            if not (self.operator == "." or self.operator == "["):
                raise Exception(str(self.operator)+" on the left side of assignment expression")
            else:
                self.left.semantic(SemanticChecker(0))
                self.right.semantic(SemanticChecker(0))
        elif self.operator == "=":
            self.left.semantic(SemanticChecker(1))
            self.right.semantic(SemanticChecker(-1))
        else:
            self.left.semantic(sc)
            self.right.semantic(sc)
    def __repr__(self):
        global loopNr
        if self.operator == "=":
            return self.left.__repr__()+self.right.__repr__()+self.operator+"\n"
        elif self.operator == "and":
            return self.left.__repr__()+self.right.__repr__()+self.operator+" "+str(loopNr-1)+"\n"
        else:
            return self.left.__repr__()+self.right.__repr__()+self.operator+"\n"

class UnaryExpression:
    def __init__(self, exp, operator):
        self.expression = exp
        self.operator = operator
    def semantic(self):
        if sc.assign == 1:
            raise Exception(str(self.operator)+" on the left side of assignment expression")
        else:
            self.expression.semantic(SemanticChecker(sc))
    def __repr__(self):
        return self.expression.__repr__()+("MINUS" if self.operator == '-' else self.operator)+"\n"

class ListType(Enum):
    PROGRAM = "PROGRAM"
    COMMA = "COMMA"
    SEMICOLON = "SEMICOLON"
    ARGS = "ARGS"
    FIELDS = "FIELDS"
    OR = "OR"

class List:
    def __init__(self, element, t):
        self.l = []
        self.type = t
        if element != None:
            self.l.append(element)
    def append(self, elem):
        self.l.append(elem)
    def semantic(self, sc):
        for a in self.l:
            a.semantic(sc)
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
        else:
            ir = ""
            for a in self.l:
                ir += a.__repr__()
            return ir+self.type.value+" "+str(len(self.l))+"\n"