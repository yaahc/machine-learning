#compiler
CC=g++

#compiler options
CFLAGS=-c -Wall -pedantic -I/usr/include/gsl

#linker options
LDFLAGS=-L/usr/local/lib -lgsl -lgslcblas -lm



all: pa1

pa1: pa1.o PatternSet.o
	$(CC) $(LDFLAGS) pa1.o PatternSet.o -o pa1

pa1.o: pa1.cpp
	$(CC) $(CFLAGS) pa1.cpp

PatternSet.o: PatternSet.cpp
	$(CC) $(CFLAGS) PatternSet.cpp

clean:
	rm -rf *o pa1
