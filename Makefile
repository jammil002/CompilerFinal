all: parser

parser.tab.c parser.tab.h:	parser.y
	bison -t -d -v -Wcounterexamples --report=all parser.y 

lex.yy.c: lexer.l parser.tab.h
	flex lexer.l 

parser: lex.yy.c parser.tab.c parser.tab.h AST.c symbolTable.c IRGeneration.c MipsGeneration.c
	gcc -o compiler parser.tab.c lex.yy.c AST.c symbolTable.c IRGeneration.c MipsGeneration.c
	./compiler test1.cmm

clean: 
	rm parser.tab.c lex.yy.c parser.tab.h parser.output compiler parser