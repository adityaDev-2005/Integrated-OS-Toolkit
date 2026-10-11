
#ifndef SCHEDULING_H
#define SCHEDULING_H

#define MAX_PROCESSES 100

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;

    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int remaining_time;
} Process;

/* Run all four CPU scheduling algorithms on one workload. */
void run_cpu_analyzer(void);

#endif
