# Variables #

CC=gcc
CFLAGS=-c -Wall

# Rules #

all: $(info [p09] Write 'make' then the rule you would like to run: 'pingpong-sleep' or 'pingpong-sleep-ddebug'.)

pingpong-sleep-ddebug: queue.o pingpong-ddebug.o pingpong-sleep.c
	$(CC) queue.o pingpong-ddebug.o pingpong-sleep.c -o output

pingpong-sleep: queue.o pingpong.o pingpong-sleep.c
	$(CC) queue.o pingpong.o pingpong-sleep.c -o output

pingpong-ddebug.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) -DDEBUG pingpong.c -o pingpong-ddebug.o

pingpong.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) pingpong.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) queue.c

clean: 
	find . -exec touch \{\} \;
	rm -rf *.o *.out output