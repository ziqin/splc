AR=ar
BISON=bison
CXX=g++
LEX=flex


BISONFLAGS=-t -d -v
CXXFLAGS=-std=c++17 -Wall -Wno-unused-function
LDFLAGS=-lfl -ly -Wl,--gc-sections

# installation
splc: install # CS323 project requirement adaptation
install: main.elf
	install -D $^ bin/splc

# command line interface
main.elf: main.o libparser.a libsemantic.a libgentac.a
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
main.o: main.cpp parser.hpp ast_dump.hpp
	$(CXX) $(CXXFLAGS) -c $<

# lexical analysis & syntax analysis
libparser.a: ast.o lex.yy.o syntax.tab.o
	$(AR) rcs $@ $^
ast.o: ast.cpp ast.hpp utils.hpp tac.hpp type.hpp syntax.tab.h
	$(CXX) $(CXXFLAGS) -c $<
lex.yy.o: lex.yy.c syntax.tab.h ast.hpp
	$(CXX) $(CXXFLAGS) -c $<
syntax.tab.o: syntax.tab.c syntax.tab.h ast.hpp syntax_err.hpp
	$(CXX) $(CXXFLAGS) -c $<
lex.yy.c: lex.l
	$(LEX) $^
syntax.tab.c syntax.tab.h: syntax.y
	$(BISON) $(BISONFLAGS) $^

# semantic analysis
libsemantic.a: ast.o type.o semantic.o
	$(AR) rcs $@ $^
type.o: type.cpp type.hpp utils.hpp
	$(CXX) $(CXXFLAGS) -c $<
semantic.o: semantic.cpp semantic.hpp ast_visitor.hpp
	$(CXX) $(CXXFLAGS) -c $<

# intermediate-code generation
libgentac.a: gen_tac.o
	$(AR) rcs $@ $^
gen_tac.o: gen_tac.cpp gen_tac.hpp tac.hpp ast_visitor.hpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	@rm -rf *.yy.c *.tab.c *.tab.h *.output *.o *.a *.elf bin/ test/*.out test/*.log sample/*.log tests/*.o tests/*.elf

test: unit-test diff-test

unit-test: libparser.a libsemantic.a
	cd tests && make test

diff-test: install
	cd test && ./diff_test.py

.PHONY: splc clean test
