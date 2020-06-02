# makefile for virtual memory manager

CC=gcc
CFLAGS=-Wall

all: vmm.o checker.o
	$(CC) $(CFLAGS) -o vmm vmm.o
	$(CC) $(CFLAGS) -o checker checker.o

vmm.o: vmm.c
	$(CC) $(CFLAGS) -c vmm.c

checker.o: checker.c
	$(CC) $(CFLAGS) -c checker.c

clean:
	rm -rf *.o
	rm -rf vmm
	rm -rf checker

