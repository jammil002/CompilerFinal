#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include "typeDefinitions.h"

// Define the types of AST nodes
typedef enum
{
    AST_PROGRAM,
    AST_DECLARATION,
    AST_ASSIGNMENT,
    AST_IF_STATEMENT,
    AST_WHILE_LOOP,
    AST_FUNCTION_CALL,
    AST_RETURN_STATEMENT,
    AST_BINARY_EXPR,
    AST_LITERAL,
    AST_VARIABLE,
    AST_FUNCTION_DECLARATION,
    AST_PARAMETER,
    AST_ARRAY_DECLARATION,
    AST_ARRAY_ACCESS,
    AST_TYPE,
    AST_UNARY_EXPR,
    AST_BLOCK,
    AST_ARGUMENTS,
    AST_PARAMETER_LIST,
    AST_UNEXPECTED
} NodeType;

typedef enum
{
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE
} OperatorType;

// Define the type of value that a node can hold
typedef union
{
    int intValue;
    double floatValue;
    char *strValue;
    TypeCode typeCode;
    OperatorType opType;
} Value;

// Define the structure of an AST node
typedef struct ASTNode
{
    NodeType type;
    Value value;
    struct ASTNode **children;
    int childCount;
} ASTNode;

// Function prototypes
ASTNode *createASTNode(NodeType type);
void addChildNode(ASTNode *parent, ASTNode *child);
void freeAST(ASTNode *node);
const char *nodeTypeToString(NodeType type);
void printIndent(int level);
void printNodeValue(ASTNode *node);
void printAST(ASTNode *node, int level);
ASTNode *createTypeNode(NodeType nodeType, TypeCode typeCode);

#endif // AST_H