# Variables #

CC=gcc
CFLAGS=-c -Wall

# Rules #

all: output

output: queue.o testafila.o
	$(CC) queue.o testafila.o -o output

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) queue.c

testafila.o: testafila.c
	$(CC) $(CFLAGS) testafila.c

clean: 
	rm -rf *.o
