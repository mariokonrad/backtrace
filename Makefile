.PHONY: all clean

CXX=g++
CXXFLAGS=-std=c++11 -Wall -Wextra -pedantic

all : backtrace

backtrace : backtrace.o
	$(CXX) -rdynamic -o $@ $^

clean :
	rm -f *.o
	rm -f backtrace

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
