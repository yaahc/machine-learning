#compiler
CC=g++

#compiler options
CFLAGS=-c -Wall -pedantic -I/usr/include/gsl

#linker options
LDFLAGS=-L/usr/local/lib -lgsl -lgslcblas -lm



all: pa2

pa2: pa2.o PatternSet.o
	$(CC) $(LDFLAGS) pa2.o PatternSet.o -o pa2

pa2.o: pa2.cpp
	$(CC) $(CFLAGS) pa2.cpp

PatternSet.o: PatternSet.cpp
	$(CC) $(CFLAGS) PatternSet.cpp

clean:
	rm -rf *o pa2
