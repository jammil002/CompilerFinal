%{
#include <stdio.h>
#include <stdlib.h>
#include "AST.h"
#include "typeDefinitions.h"
#include "symbolTable.h"

extern int yylineno;
int yyerror(const char* s);
extern int yylex(void);

SymbolTable* symbolTable;
ASTNode* astRoot;

%}

%union {
    int intValue;        // For integer values, typically used with NUMBER
    double floatValue;   // For floating-point values, used with FLOAT
    char* strValue;      // For string values, used with STRING
    char* identifier;    // For identifiers, used with IDENTIFIER
    ASTNode* astNode;    // For AST nodes
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
    { astRoot = NULL; }
    | program statement 
    {
        if ($1 == NULL) {
            astRoot = createASTNode(AST_PROGRAM);
            addChildNode(astRoot, $2);
        } else {
            addChildNode($1, $2);
        }
        $$ = $1;
    }
;

statement:
    assignment SEMICOLON
    | arrayDeclaration SEMICOLON
    | arrayAccess SEMICOLON
    | declaration SEMICOLON
    | ifStatement 
    | whileLoop 
    | functionDeclaration  
    | functionCall SEMICOLON
    | returnStatement SEMICOLON
    | expression SEMICOLON
    ;

assignment:
    IDENTIFIER ASSIGN expression {
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
    INT    { $$ = TypeINT; }
    | FLOAT  { $$ = TypeFLOAT; }
    | STRING { $$ = TypeSTRING; }
    | VOID   { $$ = TypeVOID; }
;

ifStatement:
    IF LPAREN expression RPAREN block
    {
        ASTNode* ifNode = createASTNode(AST_IF_STATEMENT);
        addChildNode(ifNode, $3);
        addChildNode(ifNode, $5);
        $$ = ifNode;
    }
    | IF LPAREN expression RPAREN block ELSE block
    {
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
        ASTNode* whileNode = createASTNode(AST_WHILE_LOOP);
        addChildNode(whileNode, $3);
        addChildNode(whileNode, $5);
        $$ = whileNode;
    }
;

functionDeclaration:
    TYPE IDENTIFIER LPAREN parameters RPAREN block
    {
        ASTNode* funcNode = createASTNode(AST_FUNCTION_DECLARATION);
        ASTNode* typeNode = createASTNode(AST_TYPE);
        typeNode->value.typeCode = $1;
        ASTNode* nameNode = createASTNode(AST_VARIABLE);
        nameNode->value.strValue = strdup($2);
        addChildNode(funcNode, typeNode);
        addChildNode(funcNode, nameNode);
        addChildNode(funcNode, $4);
        addChildNode(funcNode, $6);
        $$ = funcNode;
        addSymbolToCurrentScope(symbolTable, $2, $1);
    }
;

parameters:
    /* empty */
    {
        ASTNode* paramsNode = createASTNode(AST_PARAMETER);
        paramsNode->childCount = 0;
        $$ = paramsNode;
    }
    | parameterList
    {
        $$ = $1;
    }
;

parameterList:
    IDENTIFIER
    {
        ASTNode* paramNode = createASTNode(AST_PARAMETER);
        paramNode->value.strValue = strdup($1);
        $$ = createASTNode(AST_PARAMETER);
        addChildNode($$, paramNode);
        addSymbolToCurrentScope(symbolTable, $1, TypeUNKNOWN); 
    }
    | parameterList COMMA IDENTIFIER
    {
        ASTNode* paramNode = createASTNode(AST_PARAMETER);
        paramNode->value.strValue = strdup($3);
        addChildNode($1, paramNode);
        $$ = $1;
        addSymbolToCurrentScope(symbolTable, $3, TypeUNKNOWN);
    }
;

functionCall:
    IDENTIFIER LPAREN arguments RPAREN 
    {
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
        ASTNode* argsNode = createASTNode(AST_ARGUMENTS);
        addChildNode(argsNode, $1);
        $$ = argsNode;
    }
    | arguments COMMA expression 
    {
        addChildNode($1, $3);
        $$ = $1;
    }
;

returnStatement:
    RETURN expression 
    {
        ASTNode* returnNode = createASTNode(AST_RETURN_STATEMENT);
        addChildNode(returnNode, $2);
        $$ = returnNode;
    }
;

block:
    LBRACE
    {
        pushScope(symbolTable);
    }
    program
    RBRACE
    {
        popScope(symbolTable);
    }
;

expression:
    NUMBER
    {
        ASTNode* numNode = createASTNode(AST_LITERAL);
        numNode->value.intValue = $1;
        $$ = numNode;
    }
    | IDENTIFIER
    {
        ASTNode* idNode = createASTNode(AST_VARIABLE);
        idNode->value.strValue = strdup($1);
        $$ = idNode;
    }
    | functionCall
    {
        $$ = $1; // Assuming functionCall creates and returns an ASTNode*
    }
    | expression PLUS expression
    {
        ASTNode* exprNode = createASTNode(AST_BINARY_EXPR);
        exprNode->value.opType = OP_PLUS; // If you have an operator type in your ASTNode
        addChildNode(exprNode, $1);
        addChildNode(exprNode, $3);
        $$ = exprNode;
    }
    | expression MINUS expression
    {
        ASTNode* exprNode = createASTNode(AST_BINARY_EXPR);
        exprNode->value.opType = OP_MINUS; // Adjust similarly
        addChildNode(exprNode, $1);
        addChildNode(exprNode, $3);
        $$ = exprNode;
    }
    | expression MULTIPLY expression
    {
        ASTNode* exprNode = createASTNode(AST_BINARY_EXPR);
        exprNode->value.opType = OP_MULTIPLY; // Adjust similarly
        addChildNode(exprNode, $1);
        addChildNode(exprNode, $3);
        $$ = exprNode;
    }
    | expression DIVIDE expression
    {
        ASTNode* exprNode = createASTNode(AST_BINARY_EXPR);
        exprNode->value.opType = OP_DIVIDE; // Adjust similarly
        addChildNode(exprNode, $1);
        addChildNode(exprNode, $3);
        $$ = exprNode;
    }
    | LPAREN expression RPAREN
    {
        $$ = $2; // Just pass the expression through
    }
;

%%

int main() {   
    symbolTable = createSymbolTable(); // Initialize the symbol table
    yyparse();
    freeSymbolTable(symbolTable); // Clean up the symbol table
    return 1;
}

int yyerror(const char* s) {
    fprintf(stderr, "PARSER: Error %s at line %d\n", s, yylineno);
    return 1;
}
