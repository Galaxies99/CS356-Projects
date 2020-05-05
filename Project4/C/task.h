/**
 * Representation of a task in the system.
 */

#ifndef TASK_H
#define TASK_H

// the value of current tid.
int tid_value = 0;

// representation of a task
typedef struct task {
    char *name;
    int tid;
    int priority;
    int burst;
    // Add some extra records here to calculate the response time, etc.
    // (*) means important.
    // |-- arrival time.
    int arri_time;
    // |-- (*) waiting time.
    int wait_time;
    // |-- last time to execute.
    int last_exec;
    // |-- (*) response time.
    int resp_time;
    // |-- (*) turnaround time.
    int turn_time;
} Task;

#endif
