# Variables #

CC=gcc
CFLAGS=-c -Wall

# Rules #

all: output

output: pingpong-tasks1.o
	$(CC) pingpong-tasks1.o -o output

pingpong-tasks1.o: pingpong.c pingpong.h pingpong-tasks1.c
	$(CC) $(CFLAGS) pingpong.c pingpong-tasks1.c

clean: 
	rm -rf *.o
