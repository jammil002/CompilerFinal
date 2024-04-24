CC = gcc
LEX = flex
YACC = bison
CFLAGS = -Wall -Wextra -g

all: parser

# Compile the parser executable from Bison and Flex output, along with other C source files
parser: parser.tab.c lexer.c AST.o symbolTable.o typeDefinitions.o
	$(YACC) -d -v parser.y
	$(LEX) -o lexer.c lexer.l
	$(CC) $(CFLAGS) -o $@ parser.tab.c lexer.c AST.o symbolTable.o typeDefinitions.o

# Dependencies for object files
AST.o: AST.c AST.h
	$(CC) $(CFLAGS) -c $< -o $@

symbolTable.o: symbolTable.c symbolTable.h
	$(CC) $(CFLAGS) -c $< -o $@

typeDefinitions.o: typeDefinitions.c typeDefinitions.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f parser *.o parser.tab.c parser.tab.h lexer.c parser.output lex.yy.c test1.out

test: parser
	./parser < test1.cmm > test1.out
