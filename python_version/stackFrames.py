class FunctionFrame:
    def __init__(self, n, cl):
        self.name = n
        self.callingLine = cl
    def __repr__(self):
        return "<FN FRAME "+self.name+" "+str(self.callingLine)+">"
class IdentifierFrame:
    def __init__(self, val):
        self.value = val
    def __repr__(self):
        return "<IDE FRAME "+self.value+">"
class TupleFrame:
    def __init__(self, val):
        self.value = val
    def __repr__(self):
        output = ""
        for a in self.value:
            output += ", "+str(a)
        return "<TUPLE"+output+">"
class VoidFrame:
    def __init__(self):
        pass
class FunctionLiteral:
    def __init__(self, val):
        self.line = val
    def __repr__(self):
        return "<FUNCTION LITERAL "+str(self.line)+">"
class RetFromFunctionFrame:
    def __init__(self, val):
        self.line = val
    def __repr__(self):
        return "<RET FROM FN FRAME "+str(self.line)+">"
class EmptyFrame:
    def __init__(self):
        pass
    def __repr__(self):
        return "<EMPTY FRAME>"
class ListFrame:
    def __init__(self, inputTuple):
        self.value = inputTuple.value
    def __repr__(self):
        return "<LIST FRAME "+str(self.value)+">"
class ObjectFrame:
    def __init__(self, structName, inputDictionary):
        self.type = structName
        self.struct = inputDictionary
    def __repr__(self):
        return "<OBJECT FRAME "+self.type+" "+str(self.struct)+">"
class StructLiteral:
    def __init__(self, val):
        self.line = val
    def __repr__(self):
        return "<STRUCT LITERAL "+str(self.line)+">"
class YieldBuffer:
    def __init__(self):
        self.list = []
    def __repr__(self):
        return "<YIELD BUFFER "+str(self.list)+">"
class EveryBuffer:
    def __init__(self):
        pass
    def __repr__(self):
        return "<EVERY BUFFER>"
class FailFrame:
    def __init__(self):
        pass