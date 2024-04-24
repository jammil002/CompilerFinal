#include "MipsGeneration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_REGISTERS 10

char *registers[MAX_REGISTERS] = {"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9"};
int registerUsed[MAX_REGISTERS] = {0}; // Array to track whether a register is in use

int getAvailableRegister()
{
    for (int i = 0; i < MAX_REGISTERS; i++)
    {
        if (!registerUsed[i])
        {
            registerUsed[i] = 1; // Mark the register as used
            return i;
        }
    }
    fprintf(stderr, "Out of registers\n");
    exit(1); // Ideally, implement spill code instead of exiting
}

char *mapTempToReg(char *temp)
{
    static char tempToRegMap[100][10]; // Assuming a maximum of 100 temporaries
    static int tempCount = 0;

    // Check if this temporary has already been mapped to a register
    for (int i = 0; i < tempCount; i++)
    {
        if (strcmp(tempToRegMap[i], temp) == 0)
        {
            return strdup(registers[i % MAX_REGISTERS]);
        }
    }

    // Assign a new register to this temporary
    int regIndex = getAvailableRegister();
    if (tempCount < 100)
    {
        strcpy(tempToRegMap[tempCount], temp);
        tempCount++;
    }
    else
    {
        fprintf(stderr, "Too many temporaries\n");
        exit(1);
    }

    return strdup(registers[regIndex]);
}

void releaseRegister(char *reg)
{
    for (int i = 0; i < MAX_REGISTERS; i++)
    {
        if (strcmp(registers[i], reg) == 0)
        {
            registerUsed[i] = 0;
            return;
        }
    }
}

// Translate a single IR instruction to MIPS
void translateIRInstruction(IRInstruction *ir, FILE *outFile)
{
    char *mipsReg1, *mipsReg2, *mipsRegResult;

    // Handle different types of operations
    if (strcmp(ir->op, "+") == 0)
    {
        mipsReg1 = mapTempToReg(ir->arg1);
        mipsReg2 = mapTempToReg(ir->arg2);
        mipsRegResult = mapTempToReg(ir->result);
        fprintf(outFile, "add %s, %s, %s\n", mipsRegResult, mipsReg1, mipsReg2);
    }
    else if (strcmp(ir->op, "-") == 0)
    {
        mipsReg1 = mapTempToReg(ir->arg1);
        mipsReg2 = mapTempToReg(ir->arg2);
        mipsRegResult = mapTempToReg(ir->result);
        fprintf(outFile, "sub %s, %s, %s\n", mipsRegResult, mipsReg1, mipsReg2);
    }
    else if (strcmp(ir->op, "*") == 0)
    {
        mipsReg1 = mapTempToReg(ir->arg1);
        mipsReg2 = mapTempToReg(ir->arg2);
        mipsRegResult = mapTempToReg(ir->result);
        fprintf(outFile, "mul %s, %s, %s\n", mipsRegResult, mipsReg1, mipsReg2);
    }
    else if (strcmp(ir->op, "/") == 0)
    {
        mipsReg1 = mapTempToReg(ir->arg1);
        mipsReg2 = mapTempToReg(ir->arg2);
        mipsRegResult = mapTempToReg(ir->result);
        fprintf(outFile, "div %s, %s\n", mipsReg1, mipsReg2);
        fprintf(outFile, "mflo %s\n", mipsRegResult);
    }
    else if (strcmp(ir->op, "MOV") == 0)
    {
        mipsRegResult = mapTempToReg(ir->result);
        fprintf(outFile, "li %s, %s\n", mipsRegResult, ir->arg1);
    }
    else if (strcmp(ir->op, "LOAD") == 0)
    {
        mipsRegResult = mapTempToReg(ir->result);
        fprintf(outFile, "lw %s, 0(%s)\n", mipsRegResult, mapTempToReg(ir->arg1));
    }
    else if (strcmp(ir->op, "STORE") == 0)
    {
        mipsReg1 = mapTempToReg(ir->arg1);
        fprintf(outFile, "sw %s, 0(%s)\n", mipsReg1, mapTempToReg(ir->arg2));
    }
    else if (strcmp(ir->op, "IFGOTO") == 0 || strcmp(ir->op, "GOTO") == 0)
    {
        fprintf(outFile, "b %s\n", ir->result); // Branch to label
    }
    else if (strcmp(ir->op, "CALL") == 0)
    {
        fprintf(outFile, "jal %s\n", ir->arg1); // Jump and link to function
    }
    else if (strcmp(ir->op, "RETURN") == 0)
    {
        if (ir->arg1 != NULL)
        {
            fprintf(outFile, "move $v0, %s\n", mapTempToReg(ir->arg1)); // Move return value to $v0
        }
        fprintf(outFile, "jr $ra\n"); // Jump back to return address
    }
}

// Main function to generate MIPS from a list of IR instructions
void generateMIPS(IRInstruction *irList, const char *filename)
{
    FILE *outFile = fopen(filename, "w");
    if (!outFile)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    fprintf(outFile, ".text\n.globl main\nmain:\n");

    for (IRInstruction *current = irList; current != NULL; current = current->next)
    {
        translateIRInstruction(current, outFile);
    }

    fprintf(outFile, "jr $ra\n");
    fclose(outFile);
}
