CC      = gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Werror -pedantic

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate

assemble: assemble.o symbolTable.o utils.o

assemble.o: symbolTable.h utils.h

symbolTable.o: symbolTable.h utils.h

emulate: emulate.o utils.o

emulate.o: utils.h 

utils.o: utils.h


clean:
	rm -f $(wildcard *.o)
	rm -f assemble
	rm -f emulate
	rm -f emulate.o
	rm -f utils.o
