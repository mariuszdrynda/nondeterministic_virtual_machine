def print_bi(input):
    if isinstance(input, str):
        return len(input.value)
    else:
        return FailFrame()

def len_bi(input):
    if isinstance(input, ListFrame):
        return len(input.value)
    elif isinstance(input, TupleFrame):
        return len(input.value)
    else:
        return FailFrame()

def instanece_bi(input, string):
    if isinstance(input, ObjectFrame):
        return input.type == string
    else:
        return FailFrame()

buildInDict = {"print": print_bi, "len": len_bi, "instance" : instanece_bi}