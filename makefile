# Variables #

CC=gcc
CFLAGS=-c -Wall

# Rules #

all: $(info [p06] Write 'make' then the rule you would like to run: 'pingpong-contab' or 'pingpong-contab-prio'.)

pingpong-contab: queue.o pingpong.o pingpong-contab.c
	$(CC) queue.o pingpong.o pingpong-contab.c -o output

pingpong-contab-prio: queue.o pingpong.o pingpong-contab-prio.c
	$(CC) queue.o pingpong.o pingpong-contab-prio.c -o output

pingpong.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) pingpong.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) queue.c

clean: 
	find . -exec touch \{\} \;
	rm -rf *.o *.out output