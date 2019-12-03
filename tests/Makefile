CC=g++
CXXFLAGS=-std=c++11 -Wall
INC=-I../ -I./
LIB=-L../ -lparser -lsemantic


test: test.elf
	./$<

test.elf: test_driver.o test_type.o
	$(CC) -o $@ $(CXXFLAGS) $^ $(LIB)

test_driver.o: test_driver.cpp
	$(CC) -c $< $(CXXFLAGS) $(INC)

test_type.o: test_type.cpp
	$(CC) -c $< $(CXXFLAGS) $(INC)

clean:
	@rm -rf *.o *.elf

.PHONY: test clean
