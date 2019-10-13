CC=gcc
LEX=flex
BISON=bison

TARGET=bin/splc

CFLAGS=-Wall -Wno-unused-function
LDFLAGS=-lfl -ly
BISONFLAGS=-t -d -v

splc: parser.exe
	install -D parser.exe bin/splc
parser.exe: parser.o lex.yy.o syntax.tab.o ast.o
	$(CC) $(CFLAGS) -o parser.exe parser.o lex.yy.o syntax.tab.o ast.o $(LDFLAGS)
parser.o: parser.c syntax.tab.h ast.h
	$(CC) $(CFLAGS) -c parser.c
lex.yy.o: lex.yy.c syntax.tab.h ast.h
	$(CC) $(CFLAGS) -c lex.yy.c
syntax.tab.o: syntax.tab.c syntax.tab.h ast.h
	$(CC) $(CFLAGS) -c syntax.tab.c
ast.o: ast.c ast.h syntax.tab.h
	$(CC) $(CFLAGS) -c ast.c
lex.yy.c: lex.l
	$(LEX) lex.l
syntax.tab.c syntax.tab.h: syntax.y
	$(BISON) $(BISONFLAGS) syntax.y
clean:
	@rm -rf bin/ *.yy.c *.tab.c *.tab.h *.output *.o
.PHONY: splc
