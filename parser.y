%code requires {
#include "typeDefinitions.h"
#include "AST.h"
#include "symbolTable.h"
#include "IRGeneration.h"
#include "MipsGeneration.h"
#include <stdio.h>
#include <stdlib.h>


extern int yylineno;
int yyerror(const char* s);
extern FILE* yyin; // The input file
extern int yylex();
extern int yyparse();
extern char* yytext;

SymbolTable* symbolTable; // The symbol table   
ASTNode* astRoot; // The root of the AST
}

%start program

%union {
    int intValue;        // For integer values, typically used with NUMBER
    double floatValue;   // For floating-point values, used with FLOAT
    char* strValue;      // For string values, used with STRING
    char* identifier;    // For identifiers, used with IDENTIFIER
    struct ASTNode* astNode;    // For AST nodes
    TypeCode typeCode;   // For type codes
}

%token <intValue> NUMBER      // INTEGER literals from the lexer
%token <floatValue> FLOAT     // FLOAT literals from the lexer
%token <strValue> STRING      // STRING literals from the lexer
%token <identifier> IDENTIFIER  // Identifiers, such as variable names
%token INT VOID  // Type keywords

%type <astNode> program statement block assignment arrayDeclaration arrayAccess declaration ifStatement whileLoop functionDeclaration functionCall returnStatement expression parameters parameterList arguments
%type <typeCode> TYPE

%token PLUS MINUS MULTIPLY DIVIDE LPAREN RPAREN SEMICOLON ASSIGN 
%token LBRACKET RBRACKET IF ELSE WHILE LBRACE RBRACE COMMA RETURN

%left PLUS MINUS 
%left MULTIPLY DIVIDE



%%

program:
    /* empty */
    {
        astRoot = NULL;  // Consider setting to NULL or handling appropriately
        printf("PARSER: Empty program segment.\n");
    }
    | program statement
    {
        printf("PARSER: Adding statement to program.\n");
        if (astRoot == NULL) {
            printf("PARSER: Creating the parent program node.\n");
            astRoot = createASTNode(AST_PROGRAM);
            if (astRoot == NULL) {
                fprintf(stderr, "Error: Failed to create AST root node.\n");
                exit(1); // or handle the error in another appropriate way
            }
        }
        
        if (astRoot != NULL && $2 != NULL) {
            addChildNode(astRoot, $2);
        } else {
            if (astRoot == NULL) {
                fprintf(stderr, "Error: AST root is NULL when adding a statement.\n");
            }
            if ($2 == NULL) {
                fprintf(stderr, "Error: Statement node is NULL when adding to the program.\n");
            }
        }
        $$ = astRoot;
    }
;



statement:
    assignment SEMICOLON
    {
        $$ = $1;
        printf("PARSER: Executing statement -> assignment;\n");
    }
    | arrayDeclaration SEMICOLON
    {
        $$ = $1;
        printf("PARSER: Executing statement -> arrayDeclaration;\n");
    }
    | arrayAccess SEMICOLON
    {
        $$ = $1;
        printf("PARSER: Executing statement -> arrayAccess;\n");
    }
    | declaration SEMICOLON
    {
        $$ = $1;
        printf("PARSER: Executing statement -> declaration;\n");
    }
    | ifStatement 
    {
        $$ = $1;
        printf("PARSER: Executing statement -> ifStatement\n");
    }
    | whileLoop 
    {
        $$ = $1;
        printf("PARSER: Executing statement -> whileLoop\n");
    }
    | functionDeclaration  
    {
        $$ = $1;
        printf("PARSER: Executing statement -> functionDeclaration\n");
    }
    | functionCall 
    {
        $$ = $1;
        printf("PARSER: Executing statement -> functionCall;\n");
    }
    | returnStatement SEMICOLON
    {
        $$ = $1;
        printf("PARSER: Executing statement -> returnStatement;\n");
    }
    | expression SEMICOLON
    {
        $$ = $1;
        printf("PARSER: Executing statement -> expression;\n");
    }
;

assignment:
    IDENTIFIER ASSIGN expression {
        printf("PARSER: Executing assignment -> identifier assign expression\n");
        ASTNode* assignNode = createASTNode(AST_ASSIGNMENT);
        ASTNode* varNode = createASTNode(AST_VARIABLE);
        varNode->value.strValue = strdup($1);
        addChildNode(assignNode, varNode);
        addChildNode(assignNode, $3);
        $$ = assignNode;
        SymbolTableEntry* entry = findSymbol(symbolTable, $1);
        if (!entry) {
            printf("Undefined variable %s\n", $1);
        }
        free($1);
    }
;

arrayDeclaration:
    TYPE IDENTIFIER LBRACKET expression RBRACKET {
        printf("PARSER: Executing array declaration -> type identifier [expression]\n");
        ASTNode* arrayDeclNode = createASTNode(AST_ARRAY_DECLARATION);
        ASTNode* typeNode = createASTNode(AST_TYPE);
        typeNode->value.typeCode = $1;
        ASTNode* idNode = createASTNode(AST_VARIABLE);
        idNode->value.strValue = strdup($2);
        addChildNode(arrayDeclNode, typeNode);
        addChildNode(arrayDeclNode, idNode);
        addChildNode(arrayDeclNode, $4);
        $$ = arrayDeclNode;
        addSymbolToCurrentScope(symbolTable, $2, $1);
    }
;

arrayAccess:
    IDENTIFIER LBRACKET expression RBRACKET {
        printf("PARSER: Executing array access -> identifier[expression]\n");
        ASTNode* arrayAccessNode = createASTNode(AST_ARRAY_ACCESS);
        ASTNode* idNode = createASTNode(AST_VARIABLE);
        idNode->value.strValue = strdup($1);
        addChildNode(arrayAccessNode, idNode);
        addChildNode(arrayAccessNode, $3);
        $$ = arrayAccessNode;
    }
;

declaration:
    TYPE IDENTIFIER ASSIGN expression {
        printf("PARSER: Executing declaration with assignment -> type identifier = expression\n");
        ASTNode* declNode = createASTNode(AST_DECLARATION);
        ASTNode* typeNode = createASTNode(AST_TYPE);
        typeNode->value.typeCode = $1;
        ASTNode* idNode = createASTNode(AST_VARIABLE);
        idNode->value.strValue = strdup($2);
        addChildNode(declNode, typeNode);
        addChildNode(declNode, idNode);
        addChildNode(declNode, $4);
        $$ = declNode;
        addSymbolToCurrentScope(symbolTable, $2, $1);
    }
    | TYPE IDENTIFIER {
        printf("PARSER: Executing simple declaration -> type identifier\n");
        ASTNode* declNode = createASTNode(AST_DECLARATION);
        ASTNode* typeNode = createASTNode(AST_TYPE);
        typeNode->value.typeCode = $1;
        ASTNode* idNode = createASTNode(AST_VARIABLE);
        idNode->value.strValue = strdup($2);
        addChildNode(declNode, typeNode);
        addChildNode(declNode, idNode);
        $$ = declNode;
        addSymbolToCurrentScope(symbolTable, $2, $1);
    }
;

TYPE:
    INT    { $$ = TypeINT; printf("PARSER: Type INT recognized.\n"); }
    | FLOAT  { $$ = TypeFLOAT; printf("PARSER: Type FLOAT recognized.\n"); }
    | STRING { $$ = TypeSTRING; printf("PARSER: Type STRING recognized.\n"); }
    | VOID   { $$ = TypeVOID; printf("PARSER: Type VOID recognized.\n"); }
;

ifStatement:
    IF LPAREN expression RPAREN block
    {
        printf("PARSER: Executing IF statement -> if (expression) block\n");
        ASTNode* ifNode = createASTNode(AST_IF_STATEMENT);
        addChildNode(ifNode, $3);
        addChildNode(ifNode, $5);
        $$ = ifNode;
    }
    | IF LPAREN expression RPAREN block ELSE block
    {
        printf("PARSER: Executing IF-ELSE statement -> if (expression) block else block\n");
        ASTNode* ifElseNode = createASTNode(AST_IF_STATEMENT);
        addChildNode(ifElseNode, $3);
        addChildNode(ifElseNode, $5);
        addChildNode(ifElseNode, $7);
        $$ = ifElseNode;
    }
;

whileLoop:
    WHILE LPAREN expression RPAREN block
    {
        printf("PARSER: Executing WHILE loop -> while (expression) block\n");
        ASTNode* whileNode = createASTNode(AST_WHILE_LOOP);
        addChildNode(whileNode, $3);
        addChildNode(whileNode, $5);
        $$ = whileNode;
    }
;

functionDeclaration:
    TYPE IDENTIFIER LPAREN parameters RPAREN block
    {
        printf("PARSER: Executing functionDeclartion -> TYPE IDENTIFIER LPAREN parameters RPAREN block\n");
        ASTNode* funcNode = createASTNode(AST_FUNCTION_DECLARATION);
        ASTNode* typeNode = createASTNode(AST_TYPE);
        ASTNode* nameNode = createASTNode(AST_VARIABLE);

        typeNode->value.typeCode = $1;
        nameNode->value.strValue = strdup($2);

        addChildNode(funcNode, typeNode);
        addChildNode(funcNode, nameNode);
        addChildNode(funcNode, $4); // Assuming $4 is the parameters node
        addChildNode(funcNode, $6); // Assuming $6 is the block node

        $$ = funcNode;
        addSymbolToCurrentScope(symbolTable, $2, $1);
    }
;


parameters:
    parameterList  
    { $$ = $1; } 
    /* There's no need for the "no parameters" case anymore */
;


parameterList:
    IDENTIFIER
    {
        // Create a new parameter node
        ASTNode* paramNode = createASTNode(AST_PARAMETER);
        paramNode->value.strValue = strdup($1);

        // Create a parameter list node and add the single parameter to it
        ASTNode* paramList = createASTNode(AST_PARAMETER_LIST);
        addChildNode(paramList, paramNode);

        $$ = paramList;
    }
    | parameterList COMMA IDENTIFIER
    {
        // Create a new parameter node
        ASTNode* paramNode = createASTNode(AST_PARAMETER);
        paramNode->value.strValue = strdup($3);

        // Add the new parameter to the existing list
        addChildNode($1, paramNode);
        $$ = $1;  // Continue to use the existing parameter list
    }
;


functionCall:
    IDENTIFIER LPAREN arguments RPAREN SEMICOLON
    {
        printf("PARSER: Executing function call -> identifier(arguments)\n");
        ASTNode* callNode = createASTNode(AST_FUNCTION_CALL);
        ASTNode* nameNode = createASTNode(AST_VARIABLE);
        nameNode->value.strValue = strdup($1);
        addChildNode(callNode, nameNode);
        addChildNode(callNode, $3);
        $$ = callNode;
        free($1);
    }
;

arguments:
    expression 
    {
        printf("PARSER: Executing single argument -> expression\n");
        ASTNode* argsNode = createASTNode(AST_ARGUMENTS);
        addChildNode(argsNode, $1);
        $$ = argsNode;
    }
    | arguments COMMA expression 
    {
        printf("PARSER: Executing argument list -> arguments, expression\n");
        addChildNode($1, $3);
        $$ = $1;
    }
;

returnStatement:
    RETURN expression
    {
        printf("PARSER: Return statement with expression.\n");
        ASTNode* returnNode = createASTNode(AST_RETURN_STATEMENT);
        addChildNode(returnNode, $2); // $2 comes from 'expression'
        $$ = returnNode;
    }
    | RETURN
    {
        printf("PARSER: Return statement without expression.\n");
        ASTNode* returnNode = createASTNode(AST_RETURN_STATEMENT);
        $$ = returnNode;
    }
;

block:
    LBRACE
    {
        printf("PARSER: Executing block start -> {\n");
        pushScope(symbolTable);
    }
    program
    RBRACE
    {
        printf("PARSER: Executing block end -> }\n");
        popScope(symbolTable);
    }
;

expression:
    NUMBER
    {
        printf("PARSER: Executing expression -> number\n");
        ASTNode* numNode = createASTNode(AST_LITERAL);
        numNode->value.intValue = $1;
        $$ = numNode;
    }
    | IDENTIFIER
    {
        printf("PARSER: Executing expression -> identifier\n");
        ASTNode* idNode = createASTNode(AST_VARIABLE);
        idNode->value.strValue = strdup($1);
        $$ = idNode;
    }
    | functionCall 
    {
        printf("PARSER: Executing expression -> functionCall;\n");
        $$ = $1; 
    }
    | expression PLUS expression
    {
        printf("PARSER: Executing expression -> expression + expression\n");
        ASTNode* exprNode = createASTNode(AST_BINARY_EXPR);
        exprNode->value.opType = OP_PLUS;
        addChildNode(exprNode, $1);
        addChildNode(exprNode, $3);
        $$ = exprNode;
    }
    | expression MINUS expression
    {
        printf("PARSER: Executing expression -> expression - expression\n");
        ASTNode* exprNode = createASTNode(AST_BINARY_EXPR);
        exprNode->value.opType = OP_MINUS;
        addChildNode(exprNode, $1);
        addChildNode(exprNode, $3);
        $$ = exprNode;
    }
    | expression MULTIPLY expression
    {
        printf("PARSER: Executing expression -> expression * expression\n");
        ASTNode* exprNode = createASTNode(AST_BINARY_EXPR);
        exprNode->value.opType = OP_MULTIPLY;
        addChildNode(exprNode, $1);
        addChildNode(exprNode, $3);
        $$ = exprNode;
    }
    | expression DIVIDE expression
    {
        printf("PARSER: Executing expression -> expression / expression\n");
        ASTNode* exprNode = createASTNode(AST_BINARY_EXPR);
        exprNode->value.opType = OP_DIVIDE;
        addChildNode(exprNode, $1);
        addChildNode(exprNode, $3);
        $$ = exprNode;
    }
    | LPAREN expression RPAREN
    {
        printf("PARSER: Executing expression -> (expression)\n");
        $$ = $2; 
    }
;


%%

int main() {  
    /* extern int yydebug;
    yydebug = 1; */

    yyin = fopen("test1.cmm", "r");
    if (!yyin) {
        fprintf(stderr, "Could not open input file\n");
        return 1;
    }

    symbolTable = createSymbolTable(); // Initialize the symbol table


    if (yyparse() == 0) {
        printf("PARSER: Parsing completed successfully\n");
    } else {
        printf("PARSER: Parsing failed\n");
    }

    printf("AST: Printing AST\n");
    printAST(astRoot, 0); 

    printf("IR: Creating IR instruction\n");
    IRInstruction *irHead = generateIRForNode(astRoot);
    printIRInstructions(irHead);

    if (irHead == NULL) {
        fprintf(stderr, "Error generating IR instructions\n");
        return 1;
    }
    
    printf("MIPS: Generating MIPS code\n");
    generateMIPS(irHead, "output.asm"); // Translate the IR instructions to assembly code
    
    freeSymbolTable(symbolTable); // Clean up the symbol table
    fclose(yyin);

    return 1;
}

int yyerror(const char* s) {
    fprintf(stderr, "PARSER: Error %s at line %d near '%s'\n", s, yylineno, yytext);
    return 0;
}

