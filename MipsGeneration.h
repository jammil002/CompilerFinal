#ifndef MIPS_GENERATION_H
#define MIPS_GENERATION_H

#include <stdio.h>
#include "IRGeneration.h"

char *mapTempToReg(char *temp);
void translateIRInstruction(IRInstruction *ir, FILE *outFile);
void generateMIPS(IRInstruction *irList, const char *filename);
void releaseRegister(char *reg);
char *mapTempToReg(char *temp);
int getAvailableRegister();

#endif // MIPS_GENERATION_H
