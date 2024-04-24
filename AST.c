#include "AST.h"

// Function to create a new AST node
ASTNode *createASTNode(NodeType type)
{
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node)
    {
        perror("Failed to allocate AST node");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->children = NULL;
    node->childCount = 0;
    return node;
}

// Function to add a child node to a parent node
void addChildNode(ASTNode *parent, ASTNode *child)
{
    parent->childCount++;
    parent->children = realloc(parent->children, sizeof(ASTNode *) * parent->childCount);
    if (!parent->children)
    {
        perror("Failed to reallocate memory for children in AST node");
        exit(EXIT_FAILURE);
    }
    parent->children[parent->childCount - 1] = child;
}

// Function to free an AST
void freeAST(ASTNode *node)
{
    for (int i = 0; i < node->childCount; i++)
    {
        freeAST(node->children[i]);
    }
    free(node->children);
    free(node);
}
