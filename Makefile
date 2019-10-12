CC=gcc
LEX=flex
BISON=bison

TARGET_DIR=bin
TARGET=$(TARGET_DIR)/splc

CFLAGS=-g -Wall -Wno-unused-function
LDFLAGS=-lfl -ly
BISONFLAGS=-t -d -v

splc: lex.yy.o syntax.tab.o ast.o
	@mkdir -p $(TARGET_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) lex.yy.o syntax.tab.o ast.o $(LDFLAGS)
	@chmod +x $(TARGET)
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
