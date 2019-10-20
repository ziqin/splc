CC=gcc
LEX=flex
BISON=bison

TARGET=bin/splc

CFLAGS=-std=gnu99 -Wall -Wno-unused-function
LDFLAGS=-lfl -ly
BISONFLAGS=-t -d -v

OBJECTS=parser.o lex.yy.o syntax.tab.o ast.o syntax_errs.o

splc: parser.exe
	install -D parser.exe $(TARGET)
parser.exe: $(OBJECTS)
	$(CC) $(CFLAGS) -o parser.exe $(OBJECTS) $(LDFLAGS)
parser.o: parser.c syntax.tab.h ast.h
	$(CC) $(CFLAGS) -c parser.c
lex.yy.o: lex.yy.c syntax.tab.h ast.h
	$(CC) $(CFLAGS) -c lex.yy.c
syntax.tab.o: syntax.tab.c syntax.tab.h ast.h
	$(CC) $(CFLAGS) -c syntax.tab.c
ast.o: ast.c ast.h syntax.tab.h
	$(CC) $(CFLAGS) -c ast.c
syntax_errs.o: syntax_errs.c syntax_errs.h
	$(CC) $(CFLAGS) -c syntax_errs.c
lex.yy.c: lex.l
	$(LEX) lex.l
syntax.tab.c syntax.tab.h: syntax.y
	$(BISON) $(BISONFLAGS) syntax.y
clean:
	@rm -rf bin/ *.yy.c *.tab.c *.tab.h *.output $(OBJECTS)
.PHONY: clean
