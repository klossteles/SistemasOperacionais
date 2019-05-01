# Variables #

CC=gcc
CFLAGS=-c -Wall

# Rules #

all: output

output: queue.o pingpong.o pingpong-scheduler.o
	$(CC) queue.o pingpong.o pingpong-scheduler.o -o output

pingpong-scheduler.o: pingpong-scheduler.c
	$(CC) $(CFLAGS) pingpong-scheduler.c

pingpong.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) pingpong.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) queue.c

clean: 
	rm -rf *.o *.out output