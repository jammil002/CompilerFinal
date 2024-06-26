#pragma once

typedef enum TypeCode
{
    TypeINT,
    TypeFLOAT,
    TypeSTRING,
    TypeVOID,
    TypeBOOL,
    TypeUNKNOWN,
    TypeARRAY,
    TypePARAMATER
} TypeCode;

double convertIntToFloat(int intValue);
double ensureSameType(double op1, double op2, int type1, int type2);
const char *typeToString(TypeCode type);
