CC=gcc
CFLAGS=-Wall
PTHREADS=-lpthread

producer_consumer: producer_consumer.o
	$(CC) $(CFLAGS) -o producer_consumer producer_consumer.o $(PTHREADS)

producer_consumer.o: producer_consumer.c
	$(CC) $(CFLAGS) -c producer_consumer.c

clean: 
	rm -rf *.o
	rm -rf producer_consumer

