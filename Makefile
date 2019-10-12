CC=gcc
LEX=flex
BISON=bison

CFLAGS=-g -Wall -Wno-unused-function
LDFLAGS=-lfl -ly
BISONFLAGS=-t -d -v

splc:
	@mkdir bin
	touch bin/splc
	@chmod +x bin/splc

parser: lex.yy.o syntax.tab.o ast.o
	$(CC) $(CFLAGS) -o parser lex.yy.o syntax.tab.o ast.o $(LDFLAGS)
lex.yy.o: lex.yy.c syntax.tab.h ast.h
	$(CC) $(CFLAGS) -c lex.yy.c
syntax.tab.o: syntax.tab.c syntax.tab.h ast.h
	$(CC) $(CFLAGS) -c syntax.tab.c
ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c
lex.yy.c: lex.l
	$(LEX) lex.l
syntax.tab.c syntax.tab.h: syntax.y
	$(BISON) $(BISONFLAGS) syntax.y
clean:
	@rm -rf bin/ *.yy.c *.tab.c *.tab.h *.output *.o
.PHONY: splc
