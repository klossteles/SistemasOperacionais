# Variables #

CC=gcc
CFLAGS=-c -Wall

# Rules #

all: $(info [p02] Write 'make' then the rule you would like to run: 'tasks1', 'tasks2' or 'tasks3'.)

tasks1: pingpong.o pingpong-tasks1.o
	$(CC) pingpong.o pingpong-tasks1.o -o output

tasks2: pingpong.o pingpong-tasks2.o	
	$(CC) pingpong.o pingpong-tasks2.o -o output

tasks3: pingpong.o pingpong-tasks3.o
	$(CC) pingpong.o pingpong-tasks3.o -o output

pingpong.o: pingpong.c pingpong.h
	$(CC) $(CFLAGS) pingpong.c

pingpong-tasks1.o: pingpong-tasks1.c
	$(CC) $(CFLAGS) pingpong-tasks1.c

pingpong-tasks2.o: pingpong-tasks2.c
	$(CC) $(CFLAGS) pingpong-tasks2.c

pingpong-tasks3.o: pingpong-tasks3.c
	$(CC) $(CFLAGS) pingpong-tasks3.c

clean: 
	rm -rf *.o
