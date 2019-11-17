AR=ar
BISON=bison
CC=gcc
LEX=flex


BISONFLAGS=-t -d -v
CFLAGS=-std=gnu99 -Wall -Wno-unused-function -ffunction-sections -fdata-sections
LDFLAGS=-static -lfl -ly -Wl,--gc-sections

# installation
splc: install # CS323 project requirement adaptation
install: main.elf
	install -D $^ bin/splc

# command line interface
main.elf: main.o libparser.a libsementic.a
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
main.o: main.c parser.h
	$(CC) $(CFLAGS) -c $<

# lexical analysis & syntax analysis
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

# semantic analysis
libsementic.a: ast.o symbol_table.o type.o dict.o
	$(AR) rcs $@ $^
ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c $<
symbol_table.o: symbol_table.c symbol_table.h
	$(CC) $(CFLAGS) -c $<
type.o: type.c type.h utils/array.h
	$(CC) $(CFLAGS) -c $<
dict.o: utils/dict.c utils/dict.h
	$(CC) $(CFLAGS) -c $<

clean:
	@rm -rf *.yy.c *.tab.c *.tab.h *.output *.o *.a *.elf bin/
.PHONY: splc clean
