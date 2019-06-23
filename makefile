# Variables #

CC=gcc
CFLAGS=-c -Wall

# Rules #

all: $(info [p08] Write 'make' then the rule you would like to run: 'pingpong-join' or 'pingpong-join-ddebug'.)

pingpong-join-ddebug: queue.o pingpong-ddebug.o pingpong-join.c
	$(CC) queue.o pingpong-ddebug.o pingpong-join.c -o output

pingpong-join: queue.o pingpong.o pingpong-join.c
	$(CC) queue.o pingpong.o pingpong-join.c -o output

pingpong-ddebug.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) -DDEBUG pingpong.c -o pingpong-ddebug.o

pingpong.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) pingpong.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) queue.c

clean: 
	find . -exec touch \{\} \;
	rm -rf *.o *.out output