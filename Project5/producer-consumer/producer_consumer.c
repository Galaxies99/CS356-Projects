# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <pthread.h>
# include <semaphore.h>
# include "buffer.h"

# define TRUE 1

// buffer implementation: circular queue
buffer_item buf[BUFFER_SIZE + 1];
int head, tail;

// insert an item to the buffer
int insert_item(buffer_item item) {
	if ((tail + 1) % (BUFFER_SIZE + 1) == head) return -1;
	tail = (tail + 1) % (BUFFER_SIZE + 1);
	buf[tail] = item;
	return 0;
}

// remove an item from the buffer
int remove_item(buffer_item *item) {
	if (head == tail) return -1;
	head = (head + 1) % (BUFFER_SIZE + 1);
	*item = buf[head];
	return 0;
}

// semaphore empty, full
sem_t empty, full;
// mutex lock mutex
pthread_mutex_t mutex;
// terminate flag
int terminate_flag;

// producer thread
void *producer(void *param) {
	buffer_item item;
	
	while(TRUE) {
		static int time_period;
		time_period = rand() % 3;
		sleep(time_period);
		
		// generate an item
		item = rand();

		sem_wait(&empty);
		pthread_mutex_lock(&mutex);
		if (terminate_flag) break;
		
		if (insert_item(item)) {
			fprintf(stderr, "Error: producer can not insert the item!\n");
			exit(1);
		}
		else fprintf(stdout, "[Log] Producer produced item %d.\n", item);

		pthread_mutex_unlock(&mutex);
		sem_post(&full);
	}
	pthread_mutex_unlock(&mutex);
	pthread_exit(0);
}

// consumer thread
void *consumer(void *param) {
	buffer_item item;
	
	while(TRUE) {
		static int time_period;
		time_period = rand() % 3;
		sleep(time_period);

		sem_wait(&full);
		pthread_mutex_lock(&mutex);
		if (terminate_flag) break;

		if (remove_item(&item)) {
			fprintf(stderr, "Error: consumer can not remove the item!\n");
			exit(1);
		}
		else fprintf(stdout, "[Log] Consumer consumed item %d.\n", item);

		pthread_mutex_unlock(&mutex);
		sem_post(&empty);
	}
	pthread_mutex_unlock(&mutex);
	pthread_exit(0);
}

int main(int argc, char *argv[]) {
	pthread_t *producer_t, *consumer_t;
	int total_time, producer_number, consumer_number;
	static int err;

	// extract the arguments
	if(argc != 4) {
		fprintf(stderr, "Error: invalid arguments.\n");
		exit(1);
	}
	total_time = atoi(argv[1]);
	producer_number = atoi(argv[2]);
	consumer_number = atoi(argv[3]);

	// initialization
	head = 0; tail = 0;
	terminate_flag = 0;
	// |-- create the mutex lock
	err = pthread_mutex_init(&mutex, NULL);
	if (err) {
		fprintf(stderr, "Error: pthread mutex initialization error!\n");
		exit(1);
	}
	// |-- create the semaphore
	err = sem_init(&empty, 0, BUFFER_SIZE);
	if (err) {
		fprintf(stderr, "Error: semaphore initialization error!\n");
		exit(1);
	}	
	err = sem_init(&full, 0, 0);
	if(err) {
		fprintf(stderr, "Error: semaphore initialization error!\n");
		exit(1);
	}	

	// create threads
	producer_t = (pthread_t *) malloc (sizeof(pthread_t) * producer_number);
	consumer_t = (pthread_t *) malloc (sizeof(pthread_t) * consumer_number);

	for (int i = 0; i < producer_number; ++ i)
		pthread_create(&producer_t[i], NULL, &producer, NULL);
	for (int i = 0; i < consumer_number; ++ i)
		pthread_create(&consumer_t[i], NULL, &consumer, NULL);

	// sleep some time
	sleep(total_time);

	// terminate
	terminate_flag = 1;
	// |-- terminate the threads
	for (int i = 0; i < producer_number; ++ i) 
		sem_post(&empty);
	for (int i = 0; i < consumer_number; ++ i)
		sem_post(&full);
	// |-- join the threads
	for (int i = 0; i < producer_number; ++ i) {
		err = pthread_join(producer_t[i], NULL);
		if (err) {
			fprintf(stderr, "Error: pthread join error!\n");
			exit(1);
		}
	}
	for (int i = 0; i < consumer_number; ++ i) {
		err = pthread_join(consumer_t[i], NULL);		
		if (err) {
			fprintf(stderr, "Error: pthread join error!\n");
			exit(1);
		}
	}
	fprintf(stdout, "[Log] Join the threads successfully!\n");
	// |-- destroy the mutex lock
	err = pthread_mutex_destroy(&mutex);
	if (err) {
		fprintf(stderr, "Error: pthread mutex destroy error!\n");
		exit(1);
	}
	// |-- destroy the semaphores
	err = sem_destroy(&empty);
	if (err) {
		fprintf(stderr, "Error: semaphore destroy error!\n");
		exit(1);
	}
	err = sem_destroy(&full);
	if (err) {
		fprintf(stderr, "Error: semaphore destroy error!\n");
		exit(1);
	}
	// |-- free the spaces
	free(producer_t);
	free(consumer_t);

	return 0;
}
