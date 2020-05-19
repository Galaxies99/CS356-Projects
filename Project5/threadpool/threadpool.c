/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include "threadpool.h"

#define QUEUE_SIZE 20
#define NUMBER_OF_THREADS 3

#define TRUE 1

// this represents work that has to be 
// completed by a thread in the pool
typedef struct {
	void (*function)(void *p);
	void *data;
} task;

// the work queue
struct queue_node {
	task worktodo;
	struct queue_node *nxt;
} *head, *tail;

// the worker bee
pthread_t bee[NUMBER_OF_THREADS];

// the mutex lock
pthread_mutex_t queue_mutex;

// semaphore
sem_t wait_sem;

// shutdown flag
int shutdown_flag;

// Insert a task into the queue.
// returns 0 if successful or 1 otherwise.
int enqueue(task t) {
	tail -> nxt = (struct queue_node *) malloc (sizeof(struct queue_node));
	
	// fail to allocate space
	if (tail -> nxt == NULL) return 1;

	tail = tail -> nxt;
	tail -> worktodo = t;

	return 0;
}

// Remove a task from the queue.
task dequeue() {
	if (head == tail) {
		fprintf(stderr, "Error: No more work to do!\n");
		exit(1);
	}
	
	static struct queue_node *tmp;
	tmp = head; 
	head = head -> nxt;
	free(tmp);

	return head -> worktodo;
}

// the worker thread in the thread pool.
void *worker(void *param) {
	static task tsk;
	while(TRUE) {
		// wait for available task
		sem_wait(&wait_sem);

		if (shutdown_flag) break;
		
		pthread_mutex_lock(&queue_mutex);
		tsk = dequeue();
		pthread_mutex_unlock(&queue_mutex);

		// execute the task
		execute(tsk.function, tsk.data);
	}
	pthread_exit(0);
}

// Executes the task provided to the thread pool.
void execute(void (*somefunction)(void *p), void *p) {
	(*somefunction)(p);
}


// Submits work to the pool.
int pool_submit(void (*somefunction)(void *p), void *p) {
	static task tsk;
	tsk.function = somefunction;
	tsk.data = p;
	
	pthread_mutex_lock(&queue_mutex);
	int res = enqueue(tsk);
	pthread_mutex_unlock(&queue_mutex);
	
	// success, update the semaphore.
	if (res == 0)
		sem_post(&wait_sem);
	return res;
}

// Initialize the thread pool.
void pool_init(void) {
	static int err;

	shutdown_flag = 0;

	// initialize the queue
	head = (struct queue_node *) malloc (sizeof(struct queue_node));
	if (head == NULL) {
		fprintf(stderr, "Error: queue initialization error!\n");
		exit(1);
	}
	head -> nxt = NULL;
	tail = head;

	// create the mutex lock
	err = pthread_mutex_init(&queue_mutex, NULL);
	if (err) {
		fprintf(stderr, "Error: pthread mutex initialization error!\n");
		exit(1);
	}
	
	// create the semaphore
	err = sem_init(&wait_sem, 0, 0);
	if (err) {
		fprintf(stderr, "Error: semaphore initialization error!\n");
		exit(1);
	}	
	
	// create the threads
	for (int i = 0; i < NUMBER_OF_THREADS; ++ i) {
		err = pthread_create(&bee[i], NULL, worker, NULL);
		if (err) {
			fprintf(stderr, "Error: pthread create error!\n");
			exit(1);
		}
	}
	fprintf(stdout, "[Log] Create the threads successfully!\n");
}

// shutdown the thread pool
void pool_shutdown(void) {
	static int err;

	shutdown_flag = 1;

	// release the semaphore	
	for (int i = 0; i < NUMBER_OF_THREADS; ++ i)
		sem_post(&wait_sem);
	
	// join the threads
	for (int i = 0; i < NUMBER_OF_THREADS; ++ i) {
		err = pthread_join(bee[i], NULL);
		if (err) {
			fprintf(stderr, "Error: pthread join error!\n");
			exit(1);
		}
	}
	fprintf(stdout, "[Log] Join the threads successfully!\n");
	
	// destroy the mutex lock
	err = pthread_mutex_destroy(&queue_mutex);
	if (err) {
		fprintf(stderr, "Error: pthread mutex destroy error!\n");
		exit(1);
	}
	
	// destroy the semaphore
	err = sem_destroy(&wait_sem);
	if (err) {
		fprintf(stderr, "Error: semaphore destroy error!\n");
		exit(1);
	}
}
