CXX=g++
CXXFLAGS=-std=c++17 -Wall
INC=-I../ -I./
LIB=-L../ -lparser -lsemantic


test: test.elf
	./$<

test.elf: test_driver.o test_type.o test_ast.o test_utils.o test_walker.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIB)

test_driver.o: test_driver.cpp
	$(CXX) -c $< $(CXXFLAGS) $(INC)

test_utils.o: test_utils.cpp ../utils.hpp
	$(CXX) -c $< $(CXXFLAGS) $(INC)

test_type.o: test_type.cpp ../type.hpp ../type.cpp
	$(CXX) -c $< $(CXXFLAGS) $(INC)

test_ast.o: test_ast.cpp ../ast.hpp ../ast.cpp
	$(CXX) -c $< $(CXXFLAGS) $(INC)

test_walker.o: test_walker.cpp ../ast_walker.hpp
	$(CXX) -c $< $(CXXFLAGS) $(INC)

clean:
	@rm -rf *.o *.elf

.PHONY: test clean
