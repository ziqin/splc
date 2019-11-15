AR=ar
BISON=bison
CC=gcc
LEX=flex

TARGET=bin/splc

BISONFLAGS=-t -d -v
CFLAGS=-std=gnu99 -Wall -Wno-unused-function
LDFLAGS=-static -lfl -ly


splc: main.exe
	install -D $^ $(TARGET)
main.exe: main.o libparser.a
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
main.o: main.c parser.h
	$(CC) $(CFLAGS) -c $<
libparser.a: cst.o lex.yy.o syntax.tab.o
	$(AR) rcs $@ $^
lex.yy.o: lex.yy.c syntax.tab.h cst.h
	$(CC) $(CFLAGS) -c $<
syntax.tab.o: syntax.tab.c syntax.tab.h cst.h syntax_errs.h
	$(CC) $(CFLAGS) -c $<
cst.o: cst.c cst.h syntax.tab.h
	$(CC) $(CFLAGS) -c $<
lex.yy.c: lex.l
	$(LEX) $^
syntax.tab.c syntax.tab.h: syntax.y
	$(BISON) $(BISONFLAGS) $^
clean:
	@rm -rf *.yy.c *.tab.c *.tab.h *.output *.o *.a *.exe bin/
.PHONY: clean
