/**
 * Representation of a task in the system.
 */

#ifndef TASK_H
#define TASK_H

// the task identifier
extern int tid_value;

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
