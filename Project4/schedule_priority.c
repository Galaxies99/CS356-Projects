# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# include "task.h"
# include "list.h"
# include "cpu.h"
# include "schedulers.h"

struct node *head = NULL; 

// Extra variables for calculating the response time, etc.
// |-- current time.
int time = 0;
// |-- task count.
int tsk_cnt = 0;
// |-- total waiting time.
int tot_wait_time = 0;
// |-- total response time.
int tot_resp_time = 0;
// |-- total turnaround time.
int tot_turn_time = 0;

void add(char *name, int priority, int burst) {
	Task *tsk;
	tsk = (Task *) malloc (sizeof(Task));
	// avoid racing conditions on tid_value.
	tsk -> tid = __sync_fetch_and_add(&tid_value, 1);
	tsk -> name = (char *) malloc (sizeof(char) * (1 + strlen(name)));
	strcpy(tsk -> name, name);
	tsk -> priority = priority;
	tsk -> burst = burst;
	// Record some extra parameters here to calculate the response time, etc.
	tsk -> arri_time = time;
	tsk -> last_exec = time;
	tsk -> wait_time = 0;
	tsk -> resp_time = 0;
	tsk -> turn_time = 0;
	insert(&head, tsk);
}

int schedule_next() {
	if (head == NULL) return 1;
	
	struct node *cur = head, *pos = head -> next;
	while (pos != NULL) {
		if (pos -> task -> priority >= cur -> task -> priority) cur = pos;
		pos = pos -> next;
	}
	
	Task *tsk = cur -> task;
	run(tsk, tsk -> burst);
	delete(&head, tsk);

	time += tsk -> burst;

	// Update some parameters.
	// |-- last waiting time.
	int last_wait_time = time - tsk -> last_exec - tsk -> burst;
	// |-- update waiting time.	
	tsk -> wait_time += last_wait_time;
	// |-- update response time.	
	if (tsk -> last_exec == tsk -> arri_time)
		tsk -> resp_time = last_wait_time;
	// |-- update last execution time.	
	tsk -> last_exec = time;
	
	// The task is finished, update more parameters.
	// |-- update turnaround time.
	tsk -> turn_time = time - tsk -> arri_time;
	
	// Update final counting parameters.
	tsk_cnt += 1;
	tot_wait_time += tsk -> wait_time;
	tot_resp_time += tsk -> resp_time;
	tot_turn_time+= tsk -> turn_time;

	free(tsk -> name);
	free(tsk);
	return 0;
}

void print_statistics() {
	printf("[Statistics] total %d tasks.\n", tsk_cnt);
	double avg_wait_time = 1.0 * tot_wait_time / tsk_cnt;
	double avg_resp_time = 1.0 * tot_resp_time / tsk_cnt;
	double avg_turn_time = 1.0 * tot_turn_time / tsk_cnt;
	printf("  Average Waiting Time: %.6lf\n", avg_wait_time);
	printf("  Average Response Time: %.6lf\n", avg_resp_time);
	printf("  Average Turnaround Time: %.6lf\n", avg_turn_time);
}

void schedule() {
	while (schedule_next() == 0);
	print_statistics();
}
