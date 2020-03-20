%{
    #include <iostream>
    #include <vector>
    #include "node.hpp"
    using namespace std;
    NodeList* programList;
    extern int yylex (void);
    extern void yyerror (char const *);
    extern FILE *yyin;
    extern int yylineno;
%}

%define api.value.type union
%type<NodeList*> topList structList dataList case_list elseOpt expressionList typeList emptyOrArgList argList
%type<Node*> top oper lazyFunction return_statement assignment_expression 
%type<Node*> switch_statement  case_statement loop_statement if_statement breaker nondeterministic_and_expression
%type<Node*> nondeterministic_or_expression nondeterministic_limit_expression nondeterministic_not_expression
%type<Node*> logical_OR_expression logical_AND_expression inclusive_OR_expression exclusive_OR_expression
%type<Node*> binary_and_expression equality_expression relational_expression shift_expression additive_expression 
%type<Node*> multiplicative_expression unary_expression postfix_expression primary_expression topOrEmpty lambda type
%token<char*> ID STRING_LITERAL
%token<long long> I64_LITERAL
%token<char> CHAR_LITERAL
%token<double> F64_LITERAL
%token FN STRUCT DATA OPERATOR LAZY RET YIELD SWITCH CASE ARROW EVERY WHILE DO IF ELSE BREAK CONTINUE
%token NDTAND NDTOR LIMIT NDTNOT LOGOR LOGAND EQ NEQ LE GE SHL SHR
%token FALSE TRUE LAMBDA_BEGINER NOP NIL SELF SUCCESS FAIL 
%token I64 F64 STRING CHAR BOOL VOID
%token ';' '(' ')' '{' '}' '[' ']' '_' ':' '.' '-' '!' '~' '+' '*' '/' '%' '^' "&"  '<' '>' '|' ',' '='

%%

program: topList {programList = $1;}
;
    
topList: top {$$ = new NodeList(yylineno, "SEPARATOR", $1);}
| topList ';' top {$1->addNode($3); $$ = $1;}
;

top: return_statement {$$ = $1;}
| STRUCT ID '{' structList '}' {$$ = new Node(yylineno, "STRUCT", $2, $4);}
| DATA ID '{' dataList '}' {$$ = new Node(yylineno, "DATA", $2, $4);}
| lazyFunction FN ID '(' emptyOrArgList ')' ':' type '{' topList '}' {$$ = new Node(yylineno, "FN", $1, $3, $5, $8, $10);}
| lazyFunction OPERATOR oper '(' emptyOrArgList ')' ':' type '{' topList '}'
    {
        $$ = new Node(yylineno, "OPERATOR_OVERLOADING", $1, $3, $5, $8, $10);
    }
| lazyFunction OPERATOR error '(' emptyOrArgList ')' ':' type '{' topList '}'
    {
        yyerror("Syntax error in operator overloading. Bad operator.");
    }
;

structList: ID ':' type ';' {$$ = new NodeList(yylineno, "STRUCT_LIST", new Node(yylineno, $1, $3));}
| structList ID ':' type ';' {$1->addNode(new Node(yylineno, $2, $4)); $$ = $1;}
;

dataList: ID {$$ = new NodeList(yylineno, "DATA_LIST", new Node(yylineno, "ID", $1));}
| dataList ';' ID {$1->addNode(new Node(yylineno, "ID", $3)); $$ = $1;}
;

oper: '+' {$$ = new Node(yylineno, "ADD");}
| '-' {$$ = new Node(yylineno, "MINUS");}
| '.' {$$ = new Node(yylineno, "DOT");}
| '(' ')' {$$ = new Node(yylineno, "CALL");}
 //| '{' '}' {$$ = new Node(yylineno, "");}
| '[' ']' {$$ = new Node(yylineno, "ARRAY_ELEMENT");}
| '!' {$$ = new Node(yylineno, "NEG");}
| '~' {$$ = new Node(yylineno, "BITNEG");}
| '*' {$$ = new Node(yylineno, "MUL");}
| '/' {$$ = new Node(yylineno, "DIV");}
| '%' {$$ = new Node(yylineno, "MOD");}
| '>' {$$ = new Node(yylineno, "GRE");}
| '<' {$$ = new Node(yylineno, "LES");}
| '&' {$$ = new Node(yylineno, "BIT_AND");}
| '^' {$$ = new Node(yylineno, "BIT_XOR");}
| '|' {$$ = new Node(yylineno, "BIT_OR");}
| '=' {$$ = new Node(yylineno, "ASS");}
| GE {$$ = new Node(yylineno, "GEQ");}
| LE {$$ = new Node(yylineno, "LEQ");}
| SHL {$$ = new Node(yylineno, "SHL");}
| SHR {$$ = new Node(yylineno, "SHR");}
;

lazyFunction: %empty {$$ = new Node(yylineno, "NOT_LAZY");}
| LAZY {$$ = new Node(yylineno, "LAZY");}
;

return_statement: assignment_expression {$$ = $1;}
| RET assignment_expression {$$ = new Node(yylineno, "RET", $2);}
| YIELD assignment_expression {$$ = new Node(yylineno, "YIELD", $2);}
;

assignment_expression: switch_statement {$$ = $1;}
| assignment_expression '=' switch_statement {$$ = new Node(yylineno, "ASS", $1, $3);}
;

switch_statement: loop_statement {$$ = $1;}
| SWITCH '(' top ')' '{' case_list '}' {$$ = new Node(yylineno, "SWITCH", $3, $6);}
;

case_list: case_statement {$$ = new NodeList(yylineno, "CASE_LIST", $1);}
| case_list case_statement {$1->addNode($2); $$ = $1;}
;

case_statement: CASE top ARROW '{' topList '}' {$$ = new Node(yylineno, "CASE", $2, $5);}
;

loop_statement: if_statement {$$ = $1;}
| EVERY '(' top ')' '{' topList '}' {$$ = new Node(yylineno, "EVERY", $3, $6);}
| WHILE '(' top ')' '{' topList '}' {$$ = new Node(yylineno, "WHILE", $3, $6);}
| DO '{' topList '}' WHILE '(' top ')' {$$ = new Node(yylineno, "DO_WHILE", $7, $3);}

if_statement: breaker {$$ = $1;}
| IF '(' top ')' '{' topList '}' elseOpt {$$ = new Node(yylineno, "IF", $3, $6, $8);}
;

elseOpt: %empty {$$ = nullptr;} 
| ELSE '{' topList '}' {$$ = $3;}
;

breaker: expressionList
    {
        if($1->hasOneElement()) $$ = $1->giveMeOnlyElem();
        else $$ = new Node(yylineno, $1);
    }
| CONTINUE {$$ = new Node(yylineno, "CONTINUE");}
| BREAK {$$ = new Node(yylineno, "BREAK");}
;

expressionList: nondeterministic_and_expression {$$ = new NodeList(yylineno, "COMMA", $1);}
| expressionList ',' nondeterministic_and_expression {$1->addNode($3); $$ = $1;}
;

nondeterministic_and_expression: nondeterministic_or_expression {$$ = $1;}
| nondeterministic_and_expression NDTAND nondeterministic_or_expression {$$ = new Node(yylineno, "NDTAND", $1, $3);}
;
		
nondeterministic_or_expression: nondeterministic_limit_expression {$$ = $1;}
| nondeterministic_or_expression NDTOR nondeterministic_limit_expression {$$ = new Node(yylineno, "NDTOR", $1, $3);}
;

nondeterministic_limit_expression: nondeterministic_not_expression {$$ = $1;}
| nondeterministic_limit_expression LIMIT nondeterministic_not_expression {$$ = new Node(yylineno, "LIMIT", $1, $3);}
;

nondeterministic_not_expression: logical_OR_expression {$$ = $1;}
| NDTNOT logical_OR_expression {$$ = new Node(yylineno, "LOGNOT", $2);}
;

logical_OR_expression: logical_AND_expression {$$ = $1;}
| logical_OR_expression LOGOR logical_AND_expression {$$ = new Node(yylineno, "LOGOR", $1, $3);}
;

logical_AND_expression: inclusive_OR_expression {$$ = $1;}
| logical_AND_expression LOGAND inclusive_OR_expression {$$ = new Node(yylineno, "LOGAND", $1, $3);}
;

inclusive_OR_expression: exclusive_OR_expression {$$ = $1;}
| inclusive_OR_expression '|' exclusive_OR_expression {$$ = new Node(yylineno, "BIT_OR", $1, $3);}
;

exclusive_OR_expression: binary_and_expression {$$ = $1;}
| exclusive_OR_expression '^' binary_and_expression {$$ = new Node(yylineno, "BIT_XOR", $1, $3);}
;

binary_and_expression: equality_expression {$$ = $1;}
| binary_and_expression '&' equality_expression {$$ = new Node(yylineno, "BIT_AND", $1, $3);}
;

equality_expression: relational_expression {$$ = $1;}
| equality_expression EQ relational_expression {$$ = new Node(yylineno, "EQU", $1, $3);}
| equality_expression NEQ relational_expression {$$ = new Node(yylineno, "NEQ", $1, $3);}
;

relational_expression: shift_expression {$$ = $1;}
|   relational_expression '<' shift_expression {$$ = new Node(yylineno, "LES", $1, $3);}
|   relational_expression '>' shift_expression {$$ = new Node(yylineno, "GRE", $1, $3);}
|   relational_expression LE shift_expression {$$ = new Node(yylineno, "LEQ", $1, $3);}
|   relational_expression GE shift_expression {$$ = new Node(yylineno, "GEQ", $1, $3);}
;

shift_expression: additive_expression {$$ = $1;}
| shift_expression SHL additive_expression {$$ = new Node(yylineno, "SHL", $1, $3);}
| shift_expression SHR additive_expression {$$ = new Node(yylineno, "SHR", $1, $3);}
;

additive_expression: multiplicative_expression {$$ = $1;}
| additive_expression '+' multiplicative_expression {$$ = new Node(yylineno, "ADD", $1, $3);}
| additive_expression '-' multiplicative_expression {$$ = new Node(yylineno, "MIN", $1, $3);}
;

multiplicative_expression: unary_expression {$$ = $1;}
| multiplicative_expression '*' unary_expression {$$ = new Node(yylineno, "MUL", $1, $3);}
| multiplicative_expression '/' unary_expression {$$ = new Node(yylineno, "DIV", $1, $3);}
| multiplicative_expression '%' unary_expression {$$ = new Node(yylineno, "MOD", $1, $3);}
;

unary_expression: postfix_expression {$$ = $1;}
| '-' unary_expression {$$ = new Node(yylineno, "MINUS", $2);}
| '!' unary_expression {$$ = new Node(yylineno, "NEG", $2);}
| '~' unary_expression {$$ = new Node(yylineno, "BITNEG", $2);}
;

postfix_expression: primary_expression {$$ = $1;}
| postfix_expression '.' ID {$$ = new Node(yylineno, "DOT", $1, new Node(yylineno, "ID",$1));}
| postfix_expression '[' topOrEmpty ']' {$$ = new Node(yylineno, "ARRAY_ELEMENT", $1, $3);}
| postfix_expression '(' topOrEmpty ')' {$$ = new Node(yylineno, "CALL", $1, $3);}
 //| postfix_expression '{' topOrEmpty '}' {$$ = new Node(yylineno, "{}", $1, $3);}
| postfix_expression error {yyerror("Unexpected element after postfix expression.");}
| postfix_expression '[' error ']' {yyerror("Wrong array index.");}
;

primary_expression: FALSE {$$ = new Node(yylineno, "FALSE");}
| TRUE {$$ = new Node(yylineno, "TRUE");}
| ID {$$ = new Node(yylineno, "ID", $1);}
| NIL {$$=new Node(yylineno, "NIL");}
| SELF {$$=new Node(yylineno, "SELF");}
| I64_LITERAL {$$ = new Node(yylineno, "I64", $1);}
| CHAR_LITERAL {$$ = new Node(yylineno, "CHAR", $1);}
| F64_LITERAL {$$ = new Node(yylineno, "F64", $1);}
| STRING_LITERAL {$$ = new Node(yylineno, "STRING", $1);}
| lambda {$$=$1;}
| SUCCESS {$$=new Node(yylineno, "SUCCESS");}
| FAIL {$$=new Node(yylineno, "FAIL");}
| NOP {$$=new Node(yylineno, "NOP");}
| '_' {$$=new Node(yylineno, "UNDERSCORE");}
| '(' top ')' {$$=$2;}
| '{' topList '}' {$$ = new Node(yylineno, "OBJECT", $2);}
| '[' topList ']' {$$ = new Node(yylineno, "LIST", $2);}
;

topOrEmpty: top {$$=$1;}
| %empty {$$ = new Node(yylineno, "EMPTY");}
;

lambda: LAMBDA_BEGINER '(' emptyOrArgList ')' ':' type '{' topList '}' {$$ = new Node(yylineno, "LAMBDA", $6, $3, $8);}
;

type: ID {$$ = new Node(yylineno, "TYPE_ID",$1);}
| I64 {$$ = new Node(yylineno, "TYPE_I64");}
| F64 {$$ = new Node(yylineno, "TYPE_F64");}
| STRING {$$ = new Node(yylineno, "TYPE_STRING");}
| CHAR {$$ = new Node(yylineno, "TYPE_CHAR");}
| BOOL {$$ = new Node(yylineno, "TYPE_BOOL");}
| VOID {$$ = new Node(yylineno, "TYPE_VOID");}
| '[' type ']' {$$ = new Node(yylineno, "TYPE_ARRAY", $2);}
| '{' typeList '}' {$$ = new Node(yylineno, "TYPE_OBJECT", $2);}
| '(' typeList ':' type ')' {$$ = new Node(yylineno, "TYPE_FN", $4, $2);}
;

typeList: type {$$ = new NodeList(yylineno, "TYPELIST", $1);}
| typeList ',' type {$1->addNode($3); $$ = $1;}
;

emptyOrArgList: %empty {$$ = new NodeList(yylineno, "ARGLIST", new Node(yylineno, "EMPTY"));}
| argList {$$=$1;}

argList: ID ':' type {$$ = new NodeList(yylineno, "ARGLIST", new Node(yylineno, "ARG", $1, $3));}
| argList ',' ID ':' type {$1->addNode(new Node(yylineno, "ARG", $3, $5)); $$ = $1;}
;

%%

void yyerror (char const *s){
    fprintf(stderr, "Error in line %d. %s\n", yylineno, s);
}
int main(int argc, char** arg){
    FILE* pt = fopen(arg[1], "r");
    if(!pt){
    	cout << "Error. File does not exist." << endl;
    	return -1;
    }
    yyin = pt;
    do{
        yyparse();
    }while (!feof(yyin));
    cout<<"\tPARSE DONE!\n";
    cout<<(programList->printList())<<'\n';
    return 0;
}