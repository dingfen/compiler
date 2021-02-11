CC=g++
CFLAG=-std=c++11
INC=-I include/

SRC=$(shell find src -name *.cpp)
SYNSRC=$(shell find  src/ -name *.cpp ! -name lexcolor.cpp)

Target: lex syntax

lex : src/lexcolor.cpp src/lex.cpp
	$(CC) $(CFLAG) -DCOLOR_TOKEN $^ $(INC) -o $@

syntax: $(SYNSRC)
	$(CC) $(CFLAG) -DSYNCOLOR_TOKEN $(SYNSRC) $(INC) -g -o $@

clean:
	rm lex