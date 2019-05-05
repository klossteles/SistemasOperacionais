# Variables #

CC=gcc
CFLAGS=-c -Wall

# Rules #

all: output

output: queue.o pingpong.o pingpong-preempcao.o
	$(CC) queue.o pingpong.o pingpong-preempcao.o -o output

pingpong-preempcao.o: pingpong-preempcao.c
	$(CC) $(CFLAGS) pingpong-preempcao.c

pingpong.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) pingpong.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) queue.c

clean: 
	# Solve warning "Clock skew detected".
	find . -exec touch \{\} \;
	# Remove files.
	rm -rf *.o *.out output