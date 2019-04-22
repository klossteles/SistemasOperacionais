# Variables #

CC=gcc
CFLAGS=-c -Wall

# Rules #

all: output

output: queue.o pingpong.o pingpong-dispatcher.o
	$(CC) queue.o pingpong.o pingpong-dispatcher.o -o output

pingpong-dispatcher.o: pingpong-dispatcher.c
	$(CC) $(CFLAGS) pingpong-dispatcher.c

pingpong.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) pingpong.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) queue.c

clean: 
	rm -rf *.o
