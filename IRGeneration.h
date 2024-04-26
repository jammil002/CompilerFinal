#ifndef IRGENERATION_H
#define IRGENERATION_H

#include "AST.h"

// Define the structure of an IR instruction
typedef struct IRInstruction
{
    char *op;                   // Operator
    char *arg1;                 // First argument
    char *arg2;                 // Second argument (if any)
    char *result;               // Result variable (for the output of the operation)
    struct IRInstruction *next; // Pointer to next instruction (for linked list)
} IRInstruction;

char *newLabel();
char *newTemp();
IRInstruction *appendInstruction(IRInstruction *list, IRInstruction *instr);
IRInstruction *generateIRForNode(ASTNode *node);

#endif
