#include "IRGeneration.h"
#include "AST.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tempCount = 0;  // Counter for generating unique temporary variable names
static int labelCount = 0; // Counter for generating unique label names

char *newLabel()
{
    char labelName[20];
    sprintf(labelName, "L%d", labelCount++);
    return strdup(labelName);
}

char *newTemp()
{
    char tempName[20];
    sprintf(tempName, "t%d", tempCount++);
    return strdup(tempName);
}

IRInstruction *appendInstruction(IRInstruction *list, IRInstruction *instr)
{
    if (!list)
        return instr;
    IRInstruction *head = list;
    while (head->next)
    {
        head = head->next;
    }
    head->next = instr;
    return list;
}

IRInstruction *generateIRForNode(ASTNode *node)
{
    if (!node)
    {
        printf(" IR: Node is NULL\n");
        return NULL;
    }

    IRInstruction *instr = NULL;
    IRInstruction *first = NULL;
    IRInstruction *last = NULL;

    printf(" IR: Generating for Node Type %d\n", node->type);

    switch (node->type)
    {
    case AST_PROGRAM:
        printf(" IR: Start Program\n");
        for (int i = 0; i < node->childCount; i++)
        {
            IRInstruction *childInstr = generateIRForNode(node->children[i]);
            first = first ? first : childInstr;
            last = appendInstruction(last, childInstr);
        }
        printf(" IR: End Program\n");
        break;

    case AST_DECLARATION:
    {
        char *variableName = node->children[1]->value.strValue;
        if (node->childCount == 3)
        { // Declaration with initialization
            printf(" IR: Declaration with initialization for %s\n", variableName);
            IRInstruction *exprInstr = generateIRForNode(node->children[2]);
            instr = malloc(sizeof(IRInstruction));
            instr->op = strdup("=");
            instr->arg1 = exprInstr->result;
            instr->arg2 = NULL;
            instr->result = strdup(variableName);
            instr->next = exprInstr;
        }
        else
        { // Declaration without initialization
            printf(" IR: Declaration without initialization for %s\n", variableName);
            instr = malloc(sizeof(IRInstruction));
            instr->op = strdup("NOP");
            instr->arg1 = NULL;
            instr->arg2 = NULL;
            instr->result = strdup(variableName);
            instr->next = NULL;
        }
    }
    break;

    case AST_ASSIGNMENT:
    {
        printf(" IR: Assignment\n");
        IRInstruction *valueInstr = generateIRForNode(node->children[2]);
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("=");
        instr->arg1 = valueInstr->result;
        instr->arg2 = NULL;
        instr->result = strdup(node->children[0]->value.strValue);
        instr->next = valueInstr;
    }
    break;

    case AST_IF_STATEMENT:
    {
        printf(" IR: IF Statement\n");
        IRInstruction *condInstr = generateIRForNode(node->children[0]);
        IRInstruction *thenInstr = generateIRForNode(node->children[1]);
        char *label = newLabel();
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("IFGOTO");
        instr->arg1 = condInstr->result;
        instr->arg2 = NULL;
        instr->result = label;
        instr->next = condInstr;
        appendInstruction(instr, thenInstr);
        printf(" IR: Jump to %s if true\n", label);

        if (node->childCount > 2)
        { // Has ELSE part
            IRInstruction *elseInstr = generateIRForNode(node->children[2]);
            appendInstruction(thenInstr, elseInstr);
            printf(" IR: ELSE part\n");
        }
    }
    break;

    case AST_WHILE_LOOP:
    {
        printf(" IR: WHILE Loop\n");
        IRInstruction *condInstr = generateIRForNode(node->children[0]);
        IRInstruction *bodyInstr = generateIRForNode(node->children[1]);
        char *loopLabel = newLabel();
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("WHILE");
        instr->arg1 = condInstr->result;
        instr->arg2 = NULL;
        instr->result = loopLabel;
        instr->next = condInstr;
        appendInstruction(instr, bodyInstr);
        printf(" IR: Loop condition at %s\n", loopLabel);
    }
    break;

    case AST_RETURN_STATEMENT:
    {
        printf(" IR: RETURN Statement\n");
        IRInstruction *retInstr = generateIRForNode(node->children[0]);
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("RETURN");
        instr->arg1 = retInstr->result;
        instr->arg2 = NULL;
        instr->result = NULL;
        instr->next = retInstr;
    }
    break;

    case AST_BINARY_EXPR:
    {
        printf(" IR: Start Binary Expression\n");
        IRInstruction *leftInstr = generateIRForNode(node->children[0]);
        IRInstruction *rightInstr = generateIRForNode(node->children[1]);
        instr = malloc(sizeof(IRInstruction));
        char *opType;
        switch (node->value.opType)
        {
        case OP_PLUS:
            opType = "+";
            break;
        case OP_MINUS:
            opType = "-";
            break;
        case OP_MULTIPLY:
            opType = "*";
            break;
        case OP_DIVIDE:
            opType = "/";
            break;
        default:
            opType = "unknown_op";
            break;
        }
        printf(" IR: Operation %s between %s and %s\n", opType, leftInstr->result, rightInstr->result);
        instr->op = strdup(opType);
        instr->arg1 = leftInstr->result;
        instr->arg2 = rightInstr->result;
        instr->result = newTemp();
        printf(" IR: Result stored in %s\n", instr->result);
        instr->next = NULL;

        appendInstruction(leftInstr, rightInstr);
        appendInstruction(leftInstr, instr);
        first = leftInstr;
    }
    break;

    case AST_LITERAL:
    {
        printf(" IR: Literal value %d\n", node->value.intValue);
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("MOV");
        char *literalValue = malloc(20);
        sprintf(literalValue, "%d", node->value.intValue);
        instr->arg1 = literalValue;
        instr->arg2 = NULL;
        instr->result = newTemp();
        printf(" IR: Move literal %s to %s\n", literalValue, instr->result);
        instr->next = NULL;
        first = instr;
    }
    break;

    case AST_VARIABLE:
    {
        printf(" IR: Variable access %s\n", node->value.strValue);
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("LOAD");
        instr->arg1 = strdup(node->value.strValue);
        instr->arg2 = NULL;
        instr->result = newTemp();
        printf(" IR: Load variable %s into %s\n", node->value.strValue, instr->result);
        instr->next = NULL;
        first = instr;
    }
    break;

    case AST_FUNCTION_CALL:
    {
        printf(" IR: Function call %s\n", node->children[0]->value.strValue);
        IRInstruction *argInstr = NULL;
        for (int i = 0; i < node->childCount - 1; i++)
        {
            IRInstruction *tempInstr = generateIRForNode(node->children[i + 1]);
            if (i == 0)
            {
                argInstr = tempInstr;
            }
            else
            {
                appendInstruction(argInstr, tempInstr);
            }
        }
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("CALL");
        instr->arg1 = strdup(node->children[0]->value.strValue);
        instr->arg2 = NULL;
        instr->result = newTemp();
        printf(" IR: Call result stored in %s\n", instr->result);
        instr->next = argInstr;
        first = instr;
    }
    break;

    case AST_PARAMETER:
    {
        printf(" IR: Parameter %s - no IR generated here\n", node->value.strValue);
        // Parameters usually don't generate IR directly unless for function prologues
        instr = NULL; // No IR generated here, handled at function declaration
    }
    break;

    case AST_ARRAY_DECLARATION:
    {
        printf(" IR: Allocating array %s\n", node->children[1]->value.strValue);
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("ALLOC_ARRAY");
        instr->arg1 = strdup(node->children[1]->value.strValue);    // Variable name
        instr->arg2 = generateIRForNode(node->children[2])->result; // Size expression
        instr->result = NULL;
        instr->next = NULL;
        first = instr;
    }
    break;

    case AST_ARRAY_ACCESS:
    {
        printf(" IR: Accessing array %s\n", node->children[0]->value.strValue);
        IRInstruction *indexInstr = generateIRForNode(node->children[1]);
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("ARRAY_ACCESS");
        instr->arg1 = strdup(node->children[0]->value.strValue); // Array name
        instr->arg2 = indexInstr->result;                        // Index
        instr->result = newTemp();
        instr->next = indexInstr;
        first = instr;
    }
    break;

    case AST_TYPE:
    {
        printf(" IR: Type node - no IR generated\n");
        instr = NULL; // No IR generated here
    }
    break;

    case AST_UNARY_EXPR:
    {
        printf(" IR: Unary expression with operator %s\n", node->value.opType == OP_NEGATE ? "NEG" : "NOT");
        IRInstruction *operandInstr = generateIRForNode(node->children[0]);
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup(node->value.opType == OP_NEGATE ? "NEG" : "NOT"); // Simplified unary operations
        instr->arg1 = operandInstr->result;
        instr->arg2 = NULL;
        instr->result = newTemp();
        instr->next = operandInstr;
        first = instr;
    }
    break;

    case AST_BLOCK:
    {
        printf(" IR: Entering new block scope\n");
        IRInstruction *enterScopeInstr = malloc(sizeof(IRInstruction));
        enterScopeInstr->op = strdup("ENTER_SCOPE");
        enterScopeInstr->arg1 = NULL;
        enterScopeInstr->arg2 = NULL;
        enterScopeInstr->result = NULL;
        enterScopeInstr->next = NULL;

        IRInstruction *lastInstr = enterScopeInstr;
        for (int i = 0; i < node->childCount; i++)
        {
            IRInstruction *childInstr = generateIRForNode(node->children[i]);
            lastInstr->next = childInstr;
            while (lastInstr->next)
            {
                lastInstr = lastInstr->next;
            }
        }

        IRInstruction *exitScopeInstr = malloc(sizeof(IRInstruction));
        exitScopeInstr->op = strdup("EXIT_SCOPE");
        exitScopeInstr->arg1 = NULL;
        exitScopeInstr->arg2 = NULL;
        exitScopeInstr->result = NULL;
        exitScopeInstr->next = NULL;
        lastInstr->next = exitScopeInstr;

        printf(" IR: Exiting block scope\n");
        first = enterScopeInstr;
    }
    break;

    case AST_ARGUMENTS:
    {
        printf(" IR: Generating arguments list\n");
        for (int i = 0; i < node->childCount; i++)
        {
            IRInstruction *argInstr = generateIRForNode(node->children[i]);
            if (i == 0)
            {
                first = argInstr;
            }
            else
            {
                appendInstruction(last, argInstr);
            }
            last = argInstr;
        }
    }
    break;

    case AST_FUNCTION_DECLARATION:
    {
        if (node->childCount < 4)
        {
            fprintf(stderr, "Error: Function declaration node does not have all required children.\n");
            exit(EXIT_FAILURE);
        }

        ASTNode *nameNode = node->children[1]; // The function name node.
        if (!nameNode || nameNode->type != AST_VARIABLE || !nameNode->value.strValue)
        {
            fprintf(stderr, "Error: Invalid or missing function name in function declaration.\n");
            exit(EXIT_FAILURE);
        }

        char *functionName = nameNode->value.strValue;
        printf(" IR: Function %s declaration\n", functionName);

        // Create a label for the function entry.
        IRInstruction *entryPoint = malloc(sizeof(IRInstruction));
        entryPoint->op = strdup("LABEL");
        entryPoint->arg1 = NULL;
        entryPoint->arg2 = NULL;
        entryPoint->result = strdup(functionName);
        entryPoint->next = NULL;
        printf(" IR: Label %s for function entry created\n", functionName);

        // Generate IR for the function body.
        printf(" IR: Function body for %s\n", functionName);
        IRInstruction *bodyInstr = generateIRForNode(node->children[3]); // Assuming body is the 4th child.
        if (bodyInstr)
        {
            appendInstruction(entryPoint, bodyInstr);
        }

        // Define exit point for the function.
        IRInstruction *exitPoint = malloc(sizeof(IRInstruction));
        exitPoint->op = strdup("RETURN");
        exitPoint->arg1 = exitPoint->arg2 = NULL;
        exitPoint->result = NULL;
        exitPoint->next = NULL;

        // Append exit point after the last instruction in the function body.
        IRInstruction *lastInstr = entryPoint;
        while (lastInstr && lastInstr->next)
        {
            lastInstr = lastInstr->next; // Navigate to the last instruction.
        }
        lastInstr->next = exitPoint;

        printf(" IR: Exit for function %s set up\n", functionName);
        first = entryPoint;
    }
    break;

    default:
        fprintf(stderr, "Error: Unhandled node type %d in IR generation\n", node->type);
        exit(EXIT_FAILURE);
    }

    return first;
}
