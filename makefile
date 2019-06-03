# Variables #

CC=gcc
CFLAGS=-c -Wall

# Rules #

all: $(info [p07] Write 'make' then the rule you would like to run: 'pingpong-maintask' or 'pingpong-maintask-ddebug'.)

pingpong-maintask-ddebug: queue.o pingpong-ddebug.o pingpong-maintask.c
	$(CC) queue.o pingpong-ddebug.o pingpong-maintask.c -o output

pingpong-maintask: queue.o pingpong.o pingpong-maintask.c
	$(CC) queue.o pingpong.o pingpong-maintask.c -o output

pingpong-ddebug.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) -DDEBUG pingpong.c -o pingpong-ddebug.o

pingpong.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) pingpong.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) queue.c

clean: 
	find . -exec touch \{\} \;
	rm -rf *.o *.out output