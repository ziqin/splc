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
.lex: lex.l
	$(LEX) lex.l
.syntax: syntax.y
	$(YACC) $(YAKFLAGS) syntax.y
parser: .lex .syntax
	@mkdir -p bin
	$(CC) $(CFLAGS) syntax.tab.c $(LDFLAGS) -o bin/parser
clean:
	@rm -rf bin/ *.yy.c *.tab.c *.tab.h
.PHONY: splc
