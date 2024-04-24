#include "symbolTable.h"

SymbolTable *createSymbolTable()
{
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (!table)
    {
        fprintf(stderr, "Failed to allocate memory for the symbol table\n");
        exit(EXIT_FAILURE);
    }
    table->top = NULL; // Initially, there are no scopes in the symbol table
    pushScope(table);  // Create the first (global) scope
    return table;
}

void pushScope(SymbolTable *table)
{
    Scope *newScope = (Scope *)malloc(sizeof(Scope));
    if (!newScope)
    {
        perror("Failed to allocate scope");
        exit(EXIT_FAILURE);
    }
    newScope->entries = NULL;
    newScope->next = table->top;
    table->top = newScope;
}

// Pop the top scope from the stack
void popScope(SymbolTable *table)
{
    if (table->top == NULL)
        return;

    Scope *topScope = table->top;
    table->top = topScope->next;

    SymbolTableEntry *entry = topScope->entries;
    while (entry != NULL)
    {
        SymbolTableEntry *next = entry->next;
        free(entry->identifier);
        free(entry);
        entry = next;
    }
    free(topScope);
}

// Add a symbol to the current (top) scope
void addSymbolToCurrentScope(SymbolTable *table, char *identifier, TypeCode type)
{
    if (table->top == NULL)
    {
        printf("Error: No scope in symbol table.\n");
        return;
    }

    SymbolTableEntry *newEntry = (SymbolTableEntry *)malloc(sizeof(SymbolTableEntry));
    if (!newEntry)
    {
        perror("Failed to allocate symbol table entry");
        exit(EXIT_FAILURE);
    }
    newEntry->identifier = strdup(identifier);
    newEntry->type = type;
    newEntry->next = table->top->entries;
    table->top->entries = newEntry;
}

// Find a symbol in the table starting from the current scope and moving outwards
SymbolTableEntry *findSymbol(SymbolTable *table, char *identifier)
{
    for (Scope *scope = table->top; scope != NULL; scope = scope->next)
    {
        for (SymbolTableEntry *entry = scope->entries; entry != NULL; entry = entry->next)
        {
            if (strcmp(entry->identifier, identifier) == 0)
            {
                return entry;
            }
        }
    }
    return NULL;
}

// Free the symbol table and all scopes
void freeSymbolTable(SymbolTable *table)
{
    while (table->top != NULL)
    {
        popScope(table);
    }
    free(table);
}
