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

    // if (!type)
    // {
    //     fprintf(stderr, "Error: Node type is NULL\n");
    //     exit(EXIT_FAILURE);
    // }

    node->type = type;
    node->children = NULL;
    node->childCount = 0;

    printf("AST: Node created with type %s\n", nodeTypeToString(type));
    return node;
}

// Function to add a child node to a parent node
void addChildNode(ASTNode *parent, ASTNode *child)
{
    if (!parent)
    {
        fprintf(stderr, "AST: Error -> parent is NULL\n");
        return;
    }
    if (!child)
    {
        fprintf(stderr, "AST: Error -> child is NULL\n");
        return;
    }

    printf("AST: Preparing to add child node of type %s to parent node of type %s\n",
           nodeTypeToString(child->type), nodeTypeToString(parent->type));

    parent->childCount++;
    parent->children = realloc(parent->children, sizeof(ASTNode *) * parent->childCount);
    if (!parent->children)
    {
        perror("Failed to reallocate memory for children in AST node");
        exit(EXIT_FAILURE);
    }
    parent->children[parent->childCount - 1] = child;

    printf("AST: Child node of type %s added to parent node of type %s\n",
           nodeTypeToString(child->type), nodeTypeToString(parent->type));
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

const char *nodeTypeToString(NodeType type)
{
    switch (type)
    {
    case AST_PROGRAM:
        return "AST_PROGRAM";
    case AST_DECLARATION:
        return "AST_DECLARATION";
    case AST_ASSIGNMENT:
        return "AST_ASSIGNMENT";
    case AST_IF_STATEMENT:
        return "AST_IF_STATEMENT";
    case AST_WHILE_LOOP:
        return "AST_WHILE_LOOP";
    case AST_FUNCTION_CALL:
        return "AST_FUNCTION_CALL";
    case AST_RETURN_STATEMENT:
        return "AST_RETURN_STATEMENT";
    case AST_BINARY_EXPR:
        return "AST_BINARY_EXPR";
    case AST_LITERAL:
        return "AST_LITERAL";
    case AST_VARIABLE:
        return "AST_VARIABLE";
    case AST_FUNCTION_DECLARATION:
        return "AST_FUNCTION_DECLARATION";
    case AST_PARAMETER:
        return "AST_PARAMETER";
    case AST_ARRAY_DECLARATION:
        return "AST_ARRAY_DECLARATION";
    case AST_ARRAY_ACCESS:
        return "AST_ARRAY_ACCESS";
    case AST_TYPE:
        return "AST_TYPE";
    case AST_UNARY_EXPR:
        return "AST_UNARY_EXPR";
    case AST_BLOCK:
        return "AST_BLOCK";
    case AST_ARGUMENTS:
        return "AST_ARGUMENTS";
    case AST_PARAMETER_LIST:
        return "AST_PARAMETER_LIST";
    case AST_UNEXPECTED:
        return "AST_UNEXPECTED";
    case 121:
        return "Unknown NodeType 121";
    default:
        fprintf(stderr, "Error: Unknown NodeType %d encountered\n", type);
        return "Unknown NodeType";
    }
}

// Helper function to print indents
void printIndent(int level)
{
    for (int i = 0; i < level; i++)
    {
        printf("  ");
    }
}

// Function to print the node value based on the type
void printNodeValue(ASTNode *node)
{
    switch (node->type)
    {
    case AST_LITERAL:
        printf("%d", node->value.intValue); // Assuming int literals for simplicity
        break;
    case AST_VARIABLE:
    case AST_FUNCTION_CALL:
        printf("%s", node->value.strValue);
        break;
    default:
        // For other types, no additional value to print
        break;
    }
}

void printAST(ASTNode *node, int level)
{
    if (!node)
    {
        printf("WARNING: Attempting to print a NULL node\n");
        return; // Prevent further issues by stopping here
    }

    printIndent(level);

    const char *nodeTypeStr = nodeTypeToString(node->type);
    if (nodeTypeStr)
    {
        printf("%s", nodeTypeStr);
    }
    else
    {
        printf("Unknown NodeType (%d)", node->type);
    }

    if (node->type == AST_LITERAL || node->type == AST_VARIABLE || node->type == AST_FUNCTION_CALL)
    {
        printf(" (");
        printNodeValue(node);
        printf(")");
    }
    printf("\n");

    if (!node->children)
    {
        return; // No need to iterate if there are no children
    }

    for (int i = 0; i < node->childCount; i++)
    {
        if (node->children[i] != NULL)
        {
            printAST(node->children[i], level + 1);
        }
        else
        {
            printf("WARNING: Child index %d is NULL\n", i);
        }
    }
}
