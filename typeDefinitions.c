#include <stdio.h>
#include <stdlib.h>
#include "typeDefinitions.h" // Include the type definitions

// Function to convert integer to float
double convertIntToFloat(int intType)
{
    return (double)intType;
}

// Function to ensure both operands are of the same type or can be converted
double ensureSameType(double op1, double op2, int type1, int type2)
{
    if (type1 == type2)
    {
        return op1 + op2; // Simple addition if types are the same
    }
    else if (type1 == TypeINT && type2 == TypeFLOAT)
    {
        return convertIntToFloat(op1) + op2;
    }
    else if (type1 == TypeFLOAT && type2 == TypeINT)
    {
        return op1 + convertIntToFloat(op2);
    }
    else
    {
        fprintf(stderr, "Type mismatch error\n");
        exit(1);
    }
}

// Function to print type for debugging
const char *typeToString(TypeCode type)
{
    switch (type)
    {
    case TypeINT:
        return "int";
    case TypeFLOAT:
        return "float";
    case TypeSTRING:
        return "string";
    default:
        return "unknown";
    }
}
