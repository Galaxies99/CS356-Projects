# makefile for virtual memory manager

CC=gcc
CFLAGS=-Wall

all: vmm.o
	$(CC) $(CFLAGS) -o vmm vmm.o

vmm.o: vmm.c
	$(CC) $(CFLAGS) -c vmm.c

clean:
	rm -rf *.o
	rm -rf vmm

