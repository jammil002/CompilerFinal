#ifndef MIPS_GENERATION_H
#define MIPS_GENERATION_H

#include <stdio.h>
#include "IRGeneration.h"

char *mapTempToReg(char *temp);
void translateIRInstruction(IRInstruction *ir, FILE *outFile);
void generateMIPS(IRInstruction *irList, const char *filename);

#endif // MIPS_GENERATION_H
