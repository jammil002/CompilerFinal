# Variables
CC = gcc
CFLAGS = -g -Wall
FLEX = flex
BISON = bison
EXEC = myCompiler  # Executable name

# Object files needed to link the executable
OBJS = AST.o typeDefinitions.o symbolTable.o IRGeneration.o MipsGeneration.o lexer.o parser.o

# Header dependencies
DEPS = AST.h typeDefinitions.h symbolTable.h IRGeneration.h MipsGeneration.h

# Default target
all: $(EXEC)

# Generate lexer.c from lexer.l
lexer.c: lexer.l
	$(FLEX) -o lexer.c lexer.l

# Generate parser.c and parser.h from parser.y
parser.c parser.h: parser.y
	$(BISON) -d -o parser.c parser.y

# Generic rule for compiling C files to object files
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $

# Linking all object files into the executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lm

# Clean up files
clean:
	rm -f *.o lexer.c parser.c parser.h $(EXEC)

# Rule to run the test file
run: $(EXEC)
	./$(EXEC) < test1.cmm

# Dependencies of object files on C source and generated lexer/parser code
AST.o: AST.c AST.h
	$(CC) $(CFLAGS) -c -o $@ AST.c

typeDefinitions.o: typeDefinitions.c typeDefinitions.h
	$(CC) $(CFLAGS) -c -o $@ typeDefinitions.c

symbolTable.o: symbolTable.c symbolTable.h
	$(CC) $(CFLAGS) -c -o $@ symbolTable.c

IRGeneration.o: IRGeneration.c IRGeneration.h
	$(CC) $(CFLAGS) -c -o $@ IRGeneration.c

MipsGeneration.o: MipsGeneration.c MipsGeneration.h
	$(CC) $(CFLAGS) -c -o $@ MipsGeneration.c

lexer.o: lexer.c parser.h
	$(CC) $(CFLAGS) -c -o $@ lexer.c

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c -o $@ parser.c

.PHONY: all clean run
