# Variables #

CC=gcc
CFLAGS=-c -Wall

# Rules #

all: $(info [p11] Write 'make' then the rule you would like to run: 'pingpong-barrier' or 'pingpong-barrier-ddebug'.)

pingpong-barrier-ddebug: queue.o pingpong-ddebug.o pingpong-barrier.c
	$(CC) queue.o pingpong-ddebug.o pingpong-barrier.c -o output

pingpong-barrier: queue.o pingpong.o pingpong-barrier.c
	$(CC) queue.o pingpong.o pingpong-barrier.c -o output

pingpong-ddebug.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) -DDEBUG pingpong.c -o pingpong-ddebug.o

pingpong.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) pingpong.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) queue.c

clean: 
	find . -exec touch \{\} \;
	rm -rf *.o *.out output