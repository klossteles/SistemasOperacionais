# Variables #

CC=gcc
CFLAGS=-c -Wall

# Rules #

all: $(info [p10] Write 'make' then the rule you would like to run: 'pingpong-semaphore' or 'pingpong-semaphore-ddebug'.)

pingpong-semaphore-ddebug: queue.o pingpong-ddebug.o pingpong-semaphore.c
	$(CC) queue.o pingpong-ddebug.o pingpong-semaphore.c -o output

pingpong-semaphore: queue.o pingpong.o pingpong-semaphore.c
	$(CC) queue.o pingpong.o pingpong-semaphore.c -o output

pingpong-ddebug.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) -DDEBUG pingpong.c -o pingpong-ddebug.o

pingpong.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) pingpong.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) queue.c

clean: 
	find . -exec touch \{\} \;
	rm -rf *.o *.out output