#include "IRGeneration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tempCount = 0; // Counter for generating unique temporary variable names

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
        return NULL;

    IRInstruction *instr = NULL;
    IRInstruction *first = NULL;
    IRInstruction *last = NULL;

    switch (node->type)
    {
    case AST_PROGRAM:
        for (int i = 0; i < node->childCount; i++)
        {
            IRInstruction *childInstr = generateIRForNode(node->children[i]);
            first = first ? first : childInstr;
            last = appendInstruction(last, childInstr);
        }
        break;

    case AST_DECLARATION:
        // Handles both initialized and uninitialized declarations
        {
            char *variableName = node->children[1]->value.strValue;
            if (node->childCount == 3)
            { // Declaration with initialization
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
        IRInstruction *condInstr = generateIRForNode(node->children[0]);
        IRInstruction *thenInstr = generateIRForNode(node->children[1]);
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("IFGOTO");
        instr->arg1 = condInstr->result;
        instr->arg2 = NULL;
        instr->result = newLabel(); // Create new label for jump
        instr->next = condInstr;
        appendInstruction(instr, thenInstr);

        if (node->childCount > 2)
        { // Has ELSE part
            IRInstruction *elseInstr = generateIRForNode(node->children[2]);
            appendInstruction(thenInstr, elseInstr);
        }
    }
    break;

    case AST_WHILE_LOOP:
    {
        IRInstruction *condInstr = generateIRForNode(node->children[0]);
        IRInstruction *bodyInstr = generateIRForNode(node->children[1]);
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("WHILE");
        instr->arg1 = condInstr->result;
        instr->arg2 = NULL;
        instr->result = newLabel(); // Label for loop condition
        instr->next = condInstr;
        appendInstruction(instr, bodyInstr);
    }
    break;

    case AST_FUNCTION_DECLARATION:
        // Further implementation needed
        break;

    case AST_RETURN_STATEMENT:
    {
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
        IRInstruction *leftInstr = generateIRForNode(node->children[0]);
        IRInstruction *rightInstr = generateIRForNode(node->children[1]);
        instr = malloc(sizeof(IRInstruction));
        switch (node->value.opType)
        {
        case OP_PLUS:
            instr->op = strdup("+");
            break;
        case OP_MINUS:
            instr->op = strdup("-");
            break;
        case OP_MULTIPLY:
            instr->op = strdup("*");
            break;
        case OP_DIVIDE:
            instr->op = strdup("/");
            break;
        default:
            instr->op = strdup("unknown_op");
            break;
        }
        instr->arg1 = leftInstr->result;
        instr->arg2 = rightInstr->result;
        instr->result = newTemp();
        instr->next = NULL;

        appendInstruction(leftInstr, rightInstr);
        appendInstruction(leftInstr, instr);
        first = leftInstr;
    }
    break;

    case AST_LITERAL:
    {
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("MOV");
        char *literalValue = malloc(20);
        sprintf(literalValue, "%d", node->value.intValue); // Assuming integer literals for simplicity
        instr->arg1 = literalValue;
        instr->arg2 = NULL;
        instr->result = newTemp();
        instr->next = NULL;
        first = instr;
    }
    break;

    case AST_VARIABLE:
    {
        instr = malloc(sizeof(IRInstruction));
        instr->op = strdup("LOAD");
        instr->arg1 = strdup(node->value.strValue);
        instr->arg2 = NULL;
        instr->result = newTemp();
        instr->next = NULL;
        first = instr;
    }
    break;

    case AST_FUNCTION_CALL:
    {
        IRInstruction *argInstr = NULL;
        // Generate IR for all arguments
        for (int i = 0; i < node->childCount - 1; i++)
        {
            IRInstruction *tempInstr = generateIRForNode(node->children[i + 1]); // Assuming first child is the function name
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
        instr->next = argInstr;
        first = instr;
    }
    break;

    case AST_PARAMETER:
    {
        // Parameters usually don't generate IR directly unless for function prologues
        instr = NULL; // No IR generated here, handled at function declaration
    }
    break;

    case AST_ARRAY_DECLARATION:
    {
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
        instr = NULL; // No IR generated here
    }
    break;

    case AST_UNARY_EXPR:
    {
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
        // Enter a new scope: this could translate to pushing a new frame on the stack in more complex IR
        IRInstruction *enterScopeInstr = malloc(sizeof(IRInstruction));
        enterScopeInstr->op = strdup("ENTER_SCOPE");
        enterScopeInstr->arg1 = NULL;
        enterScopeInstr->arg2 = NULL;
        enterScopeInstr->result = NULL;
        enterScopeInstr->next = NULL;

        // Generate IR for all statements within the block
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

        // Exit the scope: this could translate to popping the frame off the stack
        IRInstruction *exitScopeInstr = malloc(sizeof(IRInstruction));
        exitScopeInstr->op = strdup("EXIT_SCOPE");
        exitScopeInstr->arg1 = NULL;
        exitScopeInstr->arg2 = NULL;
        exitScopeInstr->result = NULL;
        exitScopeInstr->next = NULL;

        // Link the exit scope instruction after the last instruction of the block
        lastInstr->next = exitScopeInstr;

        // Return the first instruction (enter scope), which links through to the last (exit scope)
        first = enterScopeInstr;
    }
    break;

    case AST_ARGUMENTS:
    {

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

    default:
        fprintf(stderr, "Unhandled node type %d in IR generation\n", node->type);
        exit(EXIT_FAILURE);
    }

    return first;
}
