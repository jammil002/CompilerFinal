#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "typeDefinitions.h"

typedef struct SymbolTableEntry
{
    char *identifier;
    TypeCode type; // This could be an enum representing variable types
    struct SymbolTableEntry *next;
} SymbolTableEntry;

typedef struct Scope
{
    SymbolTableEntry *entries;
    struct Scope *next;
} Scope;

typedef struct
{
    Scope *top; // Top of the scope stack
} SymbolTable;

// Function prototypes
SymbolTable *createSymbolTable();
void pushScope(SymbolTable *table);
void popScope(SymbolTable *table);
void addSymbolToCurrentScope(SymbolTable *table, char *identifier, TypeCode type);
SymbolTableEntry *findSymbol(SymbolTable *table, char *identifier);
void freeSymbolTable(SymbolTable *table);

#endif // SYMBOL_TABLE_H