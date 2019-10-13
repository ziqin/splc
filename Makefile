CC=gcc
LEX=flex
BISON=bison

TARGET_DIR=bin
TARGET=$(TARGET_DIR)/splc

CFLAGS=-Wall -Wno-unused-function -O2
LDFLAGS=-lfl -ly
BISONFLAGS=-t -d -v

splc: main.o lex.yy.o syntax.tab.o ast.o
	@mkdir -p $(TARGET_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) main.o lex.yy.o syntax.tab.o ast.o $(LDFLAGS)
	@chmod +x $(TARGET)
main.o: main.c syntax.tab.h ast.h
	$(CC) $(CFLAGS) -c main.c
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
