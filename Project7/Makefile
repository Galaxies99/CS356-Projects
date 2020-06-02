# makefile for contiguous memory allocation

CC=gcc
CFLAGS=-Wall

all: allocator.o
	$(CC) $(CFLAGS) -o allocator allocator.o

allocator.o: allocator.c
	$(CC) $(CFLAGS) -c allocator.c

clean:
	rm -rf *.o
	rm -rf allocator

