from AST import *

reserved = {
    'struct' : 'STRUCT',
    'fn' : 'FN',
    'ret' : 'RET',
    'yield' : 'YIELD',
    'do' : 'DO',
    'while' : 'WHILE',
    'every' : 'EVERY',
    'if' : 'IF',
    'else' : 'ELSE',
    'break' : 'BREAK',
    'continue' : 'CONTINUE',
    'and' : 'AND',
    'or' : 'OR',
    'not' : 'NOT',
    'nil' : 'NIL',
    'false' : 'FALSE',
    'true' : 'TRUE'
}
tokens = [
    'F64_LITERAL',
    'I64_LITERAL',
    'CHAR_LITERAL',
    'STRING_LITERAL',
    'ID',
    'LOGOR',
    'LOGAND',
    'EQ',
    'NEQ',
    'LE',
    'GE',
    'SHL',
    'SHR'
    ] + list(reserved.values())
literals = "(){}[];-!~:*/%+><&^|,;=."

t_LOGOR = r'\|\|'
t_LOGAND = r'&&'
t_EQ = r'=='
t_NEQ = r'!='
t_LE = r'<='
t_GE = r'>='
t_SHL = r'<<'
t_SHR = r'>>'
t_CHAR_LITERAL = r'\'\'' #TODO
t_F64_LITERAL = r'\d+.\d+' #TODO
t_STRING_LITERAL = r'\"\"' #TODO

def t_I64_LITERAL(t):
    r'\d+'
    try:
        t.value = int(t.value)
    except ValueError:
        print("Integer value too large %d", t.value)
        t.value = 0
    return t

def t_ID(t):
     r'[a-zA-Z_][a-zA-Z_0-9]*' #TODO
     t.type = reserved.get(t.value,'ID')    # Check for reserved words
     return t

t_ignore = " \t"

def t_newline(t):
    r'\n+'
    t.lexer.lineno += t.value.count("\n")
    
def t_error(t):
    print("Illegal character '%s'" % t.value[0])
    t.lexer.skip(1)

import ply.lex as lex
lexer = lex.lex()

# ====================================================== PARSER ======================================================

parseList = []
start = 'program'

def p_program(t):
    '''program : topList '''
    parseList.append(t[1])
    
def p_topList(t):
    '''topList : top
        | topList ';' top
    '''
    if len(t) == 4:
        if isinstance(t[1], List):
            t[1].append(t[3])
            t[0] = t[1]
        else:
            listOfAssignment = List(t[1], ListType.SEMICOLON)
            listOfAssignment.append(t[3])
            t[0] = listOfAssignment
    else:
        t[0] = t[1]

def p_definition(t):
    '''top : functionDefinition
        | struct
        | return_statement'''
    t[0] = t[1]

def p_struct(t):
    '''struct : STRUCT ID '{' structList '}' '''
    t[0] = Struct(t[2], t[4])

def p_structList(t):
    '''structList : ID 
        | structList ',' ID
    '''
    if len(t) == 4:
        if isinstance(t[1], List):
            t[1].append(t[3])
            t[0] = t[1]
        else:
            listOfAssignment = List(t[1], ListType.FIELDS)
            listOfAssignment.append(field)
            t[0] = listOfAssignment
    else:
        t[0] = List(t[1], ListType.FIELDS)

def p_functionDefinition(t):
    '''functionDefinition : FN ID '(' argList ')' '{' topList '}' '''
    t[0] = Function(t[2], t[4], t[7])

def p_return_statement(t):
    '''return_statement : assignment_expression
        | RET assignment_expression
        | YIELD assignment_expression
    '''
    if len(t) == 2:
        t[0] = t[1]
    else:
        t[0] = Return(t[1], t[2])

def p_assignment_expression(t):
    '''assignment_expression : loop_statement
        | assignment_expression '=' loop_statement
    '''
    if len(t) == 2:
        t[0] = t[1]
    else:
        t[0] = Expression(t[1], t[3], t[2])

def p_loop_statement(t):
    '''loop_statement : if_statement
        | EVERY '(' top ')' '{' topList '}'
        | WHILE '(' top ')' '{' topList '}'
        | DO '{' topList '}' WHILE '(' top ')' '''
    if len(t) == 2:
        t[0] = t[1]
    elif len(t) == 8:
        t[0] = Statement(t[1], t[3], t[6])
    else:
        t[0] = Statement(t[1], t[7], t[3])


def p_if_statement(t):
    '''if_statement : breaker
        | IF '(' top ')' '{' topList '}' elseOpt
        '''
    if len(t) == 2:
        t[0] = t[1]
    else:
        t[0] = ifStatement(t[3], t[6], t[8])

def p_else_optional(t):
    '''elseOpt : empty
        | ELSE '{' topList '}' '''
    if len(t) == 2:
        t[0] = None
    else:
        t[0] = t[3]

def p_breaker(t):
    '''breaker : BREAK
        | CONTINUE
        | expressionList '''
    if t[1] == "break":
        t[0] = Breaker("break")
    elif t[1] == "continue":
        t[0] = Breaker("continue")
    else:
        t[0] = t[1]

def p_expressionList(t):
    '''expressionList : expressionList ',' nondeterministic_and_expression
    | nondeterministic_and_expression'''
    if len(t) == 4:
        if isinstance(t[1], List):
            t[1].append(t[3])
            t[0] = t[1]
        else:
            listOfAssignment = List(t[1], ListType.COMMA)
            listOfAssignment.append(t[3])
            t[0] = listOfAssignment
    else:
        t[0] = t[1]

def p_nondeterministic_and_expression(t):
    '''nondeterministic_and_expression : nondeterministic_or_expression
        | nondeterministic_and_expression AND nondeterministic_or_expression'''
    if len(t) > 2:
        t[0] = Expression(t[1], t[3], t[2])
    else:
        t[0] = t[1]

def p_nondeterministic_or_expression(t):
    '''nondeterministic_or_expression : nondeterministic_not_expression
        | nondeterministic_or_expression OR nondeterministic_not_expression'''
    if len(t) == 4:
        if isinstance(t[1], List):
            t[1].append(t[3])
            t[0] = t[1]
        else:
            listOfAssignment = List(t[1], ListType.OR)
            listOfAssignment.append(t[3])
            t[0] = listOfAssignment
    else:
        t[0] = t[1]

def p_nondeterministic_not_expression(t):
    '''nondeterministic_not_expression : NOT logical_OR_expression
        | logical_OR_expression'''
    if len(t) > 2:
        t[0] = Expression(t[2], None, t[1])
    else:
        t[0] = t[1]

def p_logical_OR_expression(t):
    '''logical_OR_expression : logical_AND_expression
        | logical_OR_expression LOGOR logical_AND_expression'''
    if (len(t) == 2):
         t[0] = t[1]
    else:
         t[0] = Expression(t[1], t[3], t[2])

def p_logical_AND_expression(t):
    '''logical_AND_expression : inclusive_OR_expression
        | logical_AND_expression LOGAND inclusive_OR_expression'''
    if (len(t) == 2):
         t[0] = t[1]
    else:
         t[0] = Expression(t[1], t[3], t[2])

def p_inclusive_OR_expression(t):
    '''inclusive_OR_expression : exclusive_OR_expression
        | inclusive_OR_expression '|' exclusive_OR_expression'''
    if (len(t) == 2):
         t[0] = t[1]
    else:
         t[0] = Expression(t[1], t[3], t[2])

def p_exclusive_OR_expression(t):
    '''exclusive_OR_expression : binary_and_expression
        | exclusive_OR_expression '^' binary_and_expression'''
    if (len(t) == 2):
         t[0] = t[1]
    else:
         t[0] = Expression(t[1], t[3], t[2])

def p_binary_and_expression(t):
    '''binary_and_expression : equality_expression
        | binary_and_expression "&" equality_expression'''
    if (len(t) == 2):
         t[0] = t[1]
    else:
         t[0] = Expression(t[1], t[3], t[2])

def p_equality_expression(t):
    '''equality_expression : relational_expression
        | equality_expression EQ relational_expression
        | equality_expression NEQ relational_expression'''
    if (len(t) == 2):
         t[0] = t[1]
    else:
         t[0] = Expression(t[1], t[3], t[2])

def p_relational_expression(t):
    '''relational_expression : shift_expression
    |   relational_expression '<' shift_expression
    |   relational_expression '>' shift_expression
    |   relational_expression LE shift_expression
    |   relational_expression GE shift_expression'''
    if (len(t) == 2):
         t[0] = t[1]
    else:
         t[0] = Expression(t[1], t[3], t[2])

def p_shift_expression(t):
    '''shift_expression : additive_expression
        | shift_expression SHL additive_expression
        | shift_expression SHR additive_expression '''
    if (len(t) == 2):
         t[0] = t[1]
    else:
         t[0] = Expression(t[1], t[3], t[2])

def p_additive_expression(t):
    '''additive_expression : multiplicative_expression
        | additive_expression '+' multiplicative_expression
        | additive_expression '-' multiplicative_expression'''
    if (len(t) == 2):
         t[0] = t[1]
    else:
         t[0] = Expression(t[1], t[3], t[2])

def p_multiplicative_expression(t):
    '''multiplicative_expression : unary_expression
        | multiplicative_expression '*' unary_expression
        | multiplicative_expression '/' unary_expression
        | multiplicative_expression '%' unary_expression'''
    if (len(t) == 2):
         t[0] = t[1]
    else:
         t[0] = Expression(t[1], t[3], t[2])

def p_unary_expression(t):
    '''unary_expression : postfix_expression
        | '-' unary_expression 
        | '!' unary_expression 
        | '~' unary_expression'''
    if (len(t) == 2):
         t[0] = t[1]
    else:
         t[0] = UnaryExpression(t[2], t[1])
    
def p_postfix_expression_group(t):
    '''postfix_expression : primary_expression
        | postfix_expression '.' identifier
        | postfix_expression '[' topOrEmpty ']'
        | postfix_expression '(' topOrEmpty ')'
        | postfix_expression '{' topOrEmpty '}' '''
    if (len(t) == 2):
         t[0] = t[1]
    else:
         t[0] = Expression(t[1], t[3], t[2])

def p_identifier(t):
    '''identifier : ID'''
    t[0] = Identifier(t[1])

def p_primary_expression(t):
    '''primary_expression : ID'''
    t[0] = Identifier(t[1])

def p_primary_expression_2(t):
    '''primary_expression : NIL'''
    t[0] = Literal(LiteralType.NIL, None)

def p_primary_expression_4(t):
    '''primary_expression : FALSE'''
    t[0] = Literal(LiteralType.BOOL, False)

def p_primary_expression_5(t):
    '''primary_expression : TRUE'''
    t[0] = Literal(LiteralType.BOOL, True)

def p_primary_expression_6(t):
    '''primary_expression : I64_LITERAL'''
    t[0] = Literal(LiteralType.I64_LITERAL, t[1])

def p_primary_expression_7(t):
    '''primary_expression : CHAR_LITERAL'''
    t[0] = Literal(LiteralType.CHAR_LITERAL, t[1])

def p_primary_expression_8(t):
    '''primary_expression : F64_LITERAL'''
    t[0] = Literal(LiteralType.F64_LITERAL, t[1])

def p_primary_expression_9(t):
    '''primary_expression : STRING_LITERAL'''
    t[0] = Literal(LiteralType.STRING_LITERAL, t[1])

def p_primary_expression_group(t):
    '''primary_expression : '(' top ')' '''
    t[0] = t[2]

def p_primary_expression_group_2(t):
    '''primary_expression : '[' top ']' '''
    t[0] = UnaryExpression(t[2], "MAKE_LIST")

def p_topOrEmpty(t):
    '''topOrEmpty : top
        | empty'''
    t[0] = t[1]

def p_argList(t):
    '''argList : args
    | empty '''
    if isinstance(t[1], Literal):
        t[0] = List(None, ListType.ARGS)
    else:
        t[0] = t[1]

def p_args(t):
    '''args : args ',' ID
    | ID'''
    if len(t) == 4:
        if isinstance(t[1], List):
            t[1].append(t[3])
            t[0] = t[1]
        else:
            listOfAssignment = List(t[1], ListType.ARGS)
            listOfAssignment.append(t[3])
            t[0] = listOfAssignment
    else:
        t[0] = t[1]

def p_empty(t):
    '''empty :'''
    t[0] = Literal(LiteralType.EMPTY, None)

def p_error(p):
    if p != None:
        print("Syntax error at '%s'" % p.value)
    else:
        print("Syntax error")

# ====================================================== MAIN ======================================================

import ply.yacc as yacc
parser = yacc.yacc()

def findElse(nr, ir, i):
    while True:
        if ir[i][0] == "ELSE" and ir[i][1] == nr:
            return i+1
        else:
            i += 1

def findEndIf(nr, ir, i):
    while True:
        if ir[i][0] == "ENDIF" and ir[i][1] == nr:
            return i
        else:
            i += 1

def findWhile(nr, ir, i):
    while True:
        if ir[i][0] == "while" and ir[i][1] == nr:
            return i
        else:
            i -= 1

def findEndwhile(nr, ir, i):
    while True:
        if ir[i][0] == "ENDWHILE" and ir[i][1] == nr:
            return i + 1
        else:
            i += 1

def findDoWhile(nr, ir, i):
    while True:
        if ir[i][0] == "DOWHILE" and ir[i][1] == nr:
            return i
        else:
            i -= 1

def findNextBreaker(nr, ir, i):
    while True:
        if ir[i][0] == "ENDDOWHILE" and ir[i][1] == nr:
            return i + 1
        elif ir[i][0] == "ENDWHILE" and ir[i][1] == nr:
            return i + 1
        elif ir[i][0] == "ENDEVERY" and ir[i][1] == nr:
            return i + 1
        else:
            i += 1

def findEndEvery(nr, ir, i):
    return #TODO
    while True:
        pass

def calculateJumps(ir):
    i = 0
    for a in ir:
        if len(a) > 0 and a[0] == "THEN":
            a.append(findElse(a[1], ir, i))
        if len(a) > 0 and a[0] == "ELSE":
            a.append(findEndIf(a[1], ir, i))
        if len(a) > 0 and a[0] == "DO":
            a.append(findEndwhile(a[1], ir, i))
        if len(a) > 0 and a[0] == "ENDWHILE":
            a.append(findWhile(a[1], ir, i))
        if len(a) > 0 and a[0] == "ENDDOWHILE":
            a.append(findDoWhile(a[1], ir, i))
        if len(a) > 0 and a[0] == "CASE":
            a.append(findDoWhile(a[1], ir, i))
        if len(a) > 0 and a[0] == "ARROW":
            a.append(findDoWhile(a[1], ir, i))
        if len(a) > 0 and (a[0] == "EVERYBODY" or a[0] == "and"):
            a.append(findEndEvery(a[1], ir, i))
        if len(a) > 0 and a[0] == "break":
            a.append(findNextBreaker(a[1], ir, i))
        # if len(a) > 0 and a[0] == "continue":
        #     a.append(findNextBreaker(a[1], ir, i))
        i += 1
    return ir

def generatrIR(ast):
    ir = ""
    for item in ast:
        ir += item.__repr__() + '\n'
    return [a.split() for a in ir.splitlines()]

def semantic(ir):
    #TODO sprawdź czy break i continue są w pętli
    return ir

def parserMain(inputCode):
    parser.parse(inputCode)
    return calculateJumps(semantic(generatrIR(parseList)))