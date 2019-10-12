CC=gcc
LEX=flex
YACC=bison

CFLAGS=-g -Wall -Wno-unused-function
LDFLAGS=-lfl -ly
YAKFLAGS=-t -d -v

splc:
	@mkdir bin
	touch bin/splc
	@chmod +x bin/splc

parser: lex.yy.o syntax.tab.o
	$(CC) $(CFLAGS) -o parser lex.yy.o syntax.tab.o $(LDFLAGS)
lex.yy.o: lex.yy.c syntax.tab.h
	$(CC) $(CFLAGS) -c lex.yy.c
syntax.tab.o: syntax.tab.c syntax.tab.h
	$(CC) $(CFLAGS) -c syntax.tab.c
lex.yy.c: lex.l
	$(LEX) lex.l
syntax.tab.c syntax.tab.h: syntax.y
	$(YACC) $(YAKFLAGS) syntax.y
clean:
	@rm -rf bin/ *.yy.c *.tab.c *.tab.h *.output *.o
.PHONY: splc
