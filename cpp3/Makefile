CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -I.

SRC = function.h main.cpp
OBJ = main.o

all: main

main: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

main.o: main.cpp function.h
	$(CXX) $(CXXFLAGS) -c main.cpp

tests: tests.cpp
	g++ -std=c++23 -Wall -Wextra -o tests tests.cpp -lgtest -lgtest_main -lpthread
	./tests

clean:
	rm -f main tests *.o

run: main
	./main

.PHONY: all clean run tests