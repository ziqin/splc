AR=ar
BISON=bison
CXX=g++
LEX=flex


BISONFLAGS=-t -d -v
CXXFLAGS=-std=c++11 -Wall -Wno-unused-function -ffunction-sections -fdata-sections
LDFLAGS=-static -lfl -ly -Wl,--gc-sections

# installation
splc: install # CS323 project requirement adaptation
install: main.elf
	install -D $^ bin/splc

# command line interface
main.elf: main.o libparser.a libsemantic.a
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
main.o: main.cpp parser.hpp cst.hpp
	$(CXX) $(CXXFLAGS) -c $<

# lexical analysis & syntax analysis
libparser.a: cst.o lex.yy.o syntax.tab.o
	$(AR) rcs $@ $^
lex.yy.o: lex.yy.c syntax.tab.h cst.hpp
	$(CXX) $(CXXFLAGS) -c $<
syntax.tab.o: syntax.tab.c syntax.tab.h cst.hpp syntax_errs.hpp
	$(CXX) $(CXXFLAGS) -c $<
cst.o: cst.cpp cst.hpp syntax.tab.h
	$(CXX) $(CXXFLAGS) -c $<
lex.yy.c: lex.l
	$(LEX) $^
syntax.tab.c syntax.tab.h: syntax.y
	$(BISON) $(BISONFLAGS) $^

# semantic analysis
libsemantic.a: ast.o type.o
	$(AR) rcs $@ $^
ast.o: ast.cpp ast.hpp
	$(CXX) $(CXXFLAGS) -c $<
type.o: type.cpp type.hpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	@rm -rf *.yy.c *.tab.c *.tab.h *.output *.o *.a *.elf bin/ test/*.out tests/*.o tests/*.elf

test: unit-test diff-test

unit-test: libparser.a libsemantic.a
	cd tests && make test

diff-test: install
	cd test && ./diff_test.py

.PHONY: splc clean test
