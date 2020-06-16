#TODO buildinfunction zawsze musi coś zwracać

from stackFrames import *
import math

def append_bi(list): #TODO dodaje do listy element
    pass

def print_bi(inp): #TODO
    if isinstance(inp, str):
        return len(inp.value)
    else:
        return FailFrame()

def println_bi(inp): #TODO
    if isinstance(inp, str) or isinstance(inp, int) or isinstance(inp, float):
        print(inp)
    else:
        return FailFrame()

def len_bi(inp): #TODO
    if isinstance(inp, ListFrame):
        return len(inp.value)
    elif isinstance(inp, TupleFrame):
        return len(inp.value)
    else:
        return FailFrame()

def instance_bi(inp): #TODO
    if isinstance(inp, TupleFrame) and len(inp.value) == 2:
        if isinstance(inp, ObjectFrame):
            return inp.type == string
        else:
            return FailFrame()
    else:
        return FailFrame()

def sqrt_bi(inp):
    if isinstance(inp, int):
        return int(math.sqrt(inp))
    elif isinstance(inp, float):
        return math.sqrt(inp)
    else:
        return FailFrame()

def string_bi(inp):
    return str(inp)

buildInDict = {"print": print_bi, 
                "len": len_bi,
                "instance" : instance_bi,
                "println": println_bi,
                "sqrt" : sqrt_bi,
                "string" : string_bi
            }