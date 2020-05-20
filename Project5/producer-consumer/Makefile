CC=gcc
CFLAGS=-Wall
PTHREADS=-lpthread

producer_consumer: producer_consumer.o buffer.o
	$(CC) $(CFLAGS) -o producer_consumer producer_consumer.o buffer.o $(PTHREADS)

producer_consumer.o: producer_consumer.c
	$(CC) $(CFLAGS) -c producer_consumer.c $(PTHREADS)

buffer.o: buffer.c buffer.h
	$(CC) $(CFLAG) -c buffer.c

clean: 
	rm -rf *.o
	rm -rf producer_consumer

