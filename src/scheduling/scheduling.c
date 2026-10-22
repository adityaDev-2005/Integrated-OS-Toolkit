
#include <stdio.h>
#include <string.h>

#include "benchmark.h"
#include "common.h"
#include "scheduling.h"

#define MAX_SEGMENTS 100000

typedef struct {
    int pid;
    int start;
    int end;
} Segment;

typedef struct {
    double waiting;
    double turnaround;
    double response;
} Metrics;

/* Static storage avoids putting large Gantt-chart arrays on the stack. */
static Segment timelines[4][MAX_SEGMENTS];
static int timeline_counts[4];

static int read_workload(Process p[], int *larger_priority_first)
{
    int count;
    int ordering;

    if (!read_integer("\nNumber of processes (1-100): ",
                      1, MAX_PROCESSES, &count))
        return 0;

    printf("\nPriority ordering:\n");
    printf("  1. Smaller numbers mean higher priority\n");
    printf("  2. Larger numbers mean higher priority\n");
    if (!read_integer("Select priority ordering (1-2): ", 1, 2, &ordering))
        return 0;

    *larger_priority_first = (ordering == 2);

    printf("\nEnter arrival time, burst time, and priority for each process.\n");
    printf("%s priority numbers mean higher priority.\n",
           *larger_priority_first ? "Larger" : "Smaller");

    for (int i = 0; i < count; i++) {
        char prompt[100];

        p[i] = (Process){0};
        p[i].pid = i + 1;
        p[i].response_time = -1;

        snprintf(prompt, sizeof(prompt), "P%d arrival time: ", i + 1);
        if (!read_integer(prompt, 0, 1000000, &p[i].arrival_time))
            return 0;

        snprintf(prompt, sizeof(prompt), "P%d burst time: ", i + 1);
        if (!read_integer(prompt, 1, 1000, &p[i].burst_time))
            return 0;

        snprintf(prompt, sizeof(prompt), "P%d priority: ", i + 1);
        if (!read_integer(prompt, 1, 1000000, &p[i].priority))
            return 0;

        p[i].remaining_time = p[i].burst_time;
    }


    /* Stable sort by arrival time; retain PIDs and input order for ties. */
    for (int i = 1; i < count; i++) {
        Process key = p[i];
        int j = i - 1;

        while (j >= 0 && p[j].arrival_time > key.arrival_time) {
            p[j + 1] = p[j];
            j--;
        }

        p[j + 1] = key;
    }

    return count;
}

static void add_segment(int algorithm, int pid, int start, int end)
{
    if (timeline_counts[algorithm] < MAX_SEGMENTS) {
        timelines[algorithm][timeline_counts[algorithm]++] =
            (Segment){pid, start, end};
    }
}

static void reset_results(Process p[], int count)
{
    for (int i = 0; i < count; i++) {
        p[i].completion_time = 0;
        p[i].turnaround_time = 0;
        p[i].waiting_time = 0;
        p[i].response_time = -1;
        p[i].remaining_time = p[i].burst_time;
    }
}

/* FCFS: execute processes in arrival/input order. */
static void run_fcfs(Process p[], int count)
{
    int time = 0;
    timeline_counts[0] = 0;
    reset_results(p, count);

    for (int i = 0; i < count; i++) {
        if (time < p[i].arrival_time)
            time = p[i].arrival_time;

        int start = time;
        p[i].response_time = start - p[i].arrival_time;
        time += p[i].burst_time;

        p[i].completion_time = time;
        p[i].turnaround_time = time - p[i].arrival_time;
        p[i].waiting_time = p[i].turnaround_time - p[i].burst_time;

        add_segment(0, p[i].pid, start, time);
    }
}

/* Select a ready process by shortest burst or highest priority. */
static int select_ready(Process p[], int count, int done[],
                        int time, int by_priority,
                        int larger_priority_first)
{
    int best = -1;

    for (int i = 0; i < count; i++) {
        if (done[i] || p[i].arrival_time > time)
            continue;

        if (best == -1) {
            best = i;
            continue;
        }

        int better;
        if (by_priority) {
            better = larger_priority_first
                         ? p[i].priority > p[best].priority
                         : p[i].priority < p[best].priority;
        } else {
            better = p[i].burst_time < p[best].burst_time;
        }

        /* Equal keys retain earlier arrival/input order. */
        if (better ||
            ((!by_priority ? p[i].burst_time == p[best].burst_time
                           : p[i].priority == p[best].priority) &&
             (p[i].arrival_time < p[best].arrival_time ||
              (p[i].arrival_time == p[best].arrival_time &&
               p[i].pid < p[best].pid)))) {
            best = i;
        }
    }

    return best;
}

/* SJF and Priority are both non-preemptive. */
static void run_nonpreemptive(Process p[], int count,
                              int algorithm, int by_priority,
                              int larger_priority_first)
{
    int done[MAX_PROCESSES] = {0};
    int time = 0;
    int completed = 0;

    timeline_counts[algorithm] = 0;
    reset_results(p, count);

    while (completed < count) {
        int selected = select_ready(p, count, done, time, by_priority,
                                    larger_priority_first);

        if (selected == -1) {
            int next_arrival = -1;

            for (int i = 0; i < count; i++) {
                if (!done[i] &&
                    (next_arrival == -1 ||
                     p[i].arrival_time < next_arrival))
                    next_arrival = p[i].arrival_time;
            }

            time = next_arrival;
            continue;
        }

        int start = time;
        p[selected].response_time = start - p[selected].arrival_time;
        time += p[selected].burst_time;

        p[selected].completion_time = time;
        p[selected].turnaround_time = time - p[selected].arrival_time;
        p[selected].waiting_time =
            p[selected].turnaround_time - p[selected].burst_time;

        done[selected] = 1;
        completed++;

        add_segment(algorithm, p[selected].pid, start, time);
    }
}

/* Round Robin with a circular ready queue. */
static int run_round_robin(Process p[], int count, int quantum)
{
    int queue[MAX_PROCESSES];
    int head = 0, tail = 0, size = 0;
    int next = 0, time = 0, completed = 0;

    timeline_counts[3] = 0;
    reset_results(p, count);

    while (completed < count) {
        while (next < count && p[next].arrival_time <= time) {
            queue[tail] = next;
            tail = (tail + 1) % MAX_PROCESSES;
            size++;
            next++;
        }

        if (size == 0) {
            if (next >= count)
                break;

            time = p[next].arrival_time;
            continue;
        }

        int current = queue[head];
        head = (head + 1) % MAX_PROCESSES;
        size--;

        if (p[current].response_time == -1)
            p[current].response_time = time - p[current].arrival_time;

        int start = time;
        int run_time = p[current].remaining_time < quantum
                           ? p[current].remaining_time : quantum;

        time += run_time;
        p[current].remaining_time -= run_time;

        if (timeline_counts[3] >= MAX_SEGMENTS) {
            fprintf(stderr, "Timeline capacity exceeded.\n");
            return 0;
        }
        add_segment(3, p[current].pid, start, time);

        /* New arrivals join the queue before the preempted process. */
        while (next < count && p[next].arrival_time <= time) {
            queue[tail] = next;
            tail = (tail + 1) % MAX_PROCESSES;
            size++;
            next++;
        }

        if (p[current].remaining_time > 0) {
            queue[tail] = current;
            tail = (tail + 1) % MAX_PROCESSES;
            size++;
        } else {
            p[current].completion_time = time;
            p[current].turnaround_time = time - p[current].arrival_time;
            p[current].waiting_time =
                p[current].turnaround_time - p[current].burst_time;
            completed++;
        }
    }

    return 1;
}

static Metrics print_results(Process p[], int count, int algorithm)
{
    /* Taking only an index avoids a false GCC 11 -Wstringop-overflow. */
    static const char *const names[] = {
        "FCFS",
        "SJF (NON-PREEMPTIVE)",
        "PRIORITY (NON-PREEMPTIVE)",
        "ROUND ROBIN"
    };
    double waiting = 0, turnaround = 0, response = 0;

    printf("\n========== %s ==========\n", names[algorithm]);
    printf("Timeline: ");

    for (int i = 0; i < timeline_counts[algorithm]; i++) {
        Segment s = timelines[algorithm][i];
        printf("| P%d %d-%d ", s.pid, s.start, s.end);
    }
    printf("|\n");

    printf("\nPID  AT  BT  PR  CT  TAT  WT  RT\n");

    for (int i = 0; i < count; i++) {
        printf("P%-3d %-3d %-3d %-3d %-3d %-4d %-3d %-3d\n",
               p[i].pid, p[i].arrival_time, p[i].burst_time,
               p[i].priority, p[i].completion_time,
               p[i].turnaround_time, p[i].waiting_time,
               p[i].response_time);

        waiting += p[i].waiting_time;
        turnaround += p[i].turnaround_time;
        response += p[i].response_time;
    }

    Metrics m = {
        waiting / count,
        turnaround / count,
        response / count
    };

    printf("Average waiting time:    %.2f\n", m.waiting);
    printf("Average turnaround time: %.2f\n", m.turnaround);
    printf("Average response time:   %.2f\n", m.response);

    return m;
}

static int save_csv(const Metrics results[], int quantum, int count)
{
    static const char *const fields[] = {
        "algorithm", "avg_waiting_time", "avg_turnaround_time",
        "avg_response_time", "quantum", "process_count"
    };
    static const char *const names[] = {
        "FCFS", "SJF", "Priority", "Round Robin"
    };
    BenchmarkCsv csv;

    if (!benchmark_csv_open(&csv, "cpu_scheduling.csv",
                            "CPU scheduling CSV")) {
        benchmark_report_csv_failure();
        return 0;
    }

    benchmark_csv_header(&csv, fields, sizeof(fields) / sizeof(fields[0]));

    for (int i = 0; i < 4; i++) {
        benchmark_csv_row_begin(&csv);
        benchmark_csv_text(&csv, names[i]);
        benchmark_csv_double(&csv, results[i].waiting);
        benchmark_csv_double(&csv, results[i].turnaround);
        benchmark_csv_double(&csv, results[i].response);
        benchmark_csv_int(&csv, quantum);
        benchmark_csv_int(&csv, count);
        benchmark_csv_row_end(&csv);
    }

    if (!benchmark_csv_close(&csv)) {
        benchmark_report_csv_failure();
        return 0;
    }

    benchmark_report_csv_saved(benchmark_csv_relative(&csv));
    return 1;
}

static void generate_plot_automatically(void)
{
    benchmark_generate_plot(
        "cpu_scheduling",
        "data/results/cpu_scheduling_comparison.png",
        "CPU scheduling comparison graph");
}



void run_cpu_analyzer(void)
{
    Process original[MAX_PROCESSES] = {0};
    Process work[4][MAX_PROCESSES];
    Metrics metrics[4];
    int quantum;
    int larger_priority_first = 0;

    int count = read_workload(original, &larger_priority_first);
    if (count == 0)
        return;

    if (!read_integer("Round Robin time quantum (1-1000): ",
                      1, 1000, &quantum))
        return;

    for (int i = 0; i < 4; i++)
        memcpy(work[i], original, (size_t)count * sizeof(Process));

    printf("\nRunning all four algorithms on the same workload...\n");

    run_fcfs(work[0], count);
    run_nonpreemptive(work[1], count, 1, 0, 0);
    run_nonpreemptive(work[2], count, 2, 1, larger_priority_first);

    if (!run_round_robin(work[3], count, quantum))
        return;

    metrics[0] = print_results(work[0], count, 0);
    metrics[1] = print_results(work[1], count, 1);
    metrics[2] = print_results(work[2], count, 2);
    metrics[3] = print_results(work[3], count, 3);

    printf("\n========== ALGORITHM COMPARISON ==========\n");
    printf("%-12s %-12s %-14s %-12s\n",
           "Algorithm", "Avg WT", "Avg TAT", "Avg RT");

    const char *names[] = {"FCFS", "SJF", "Priority", "Round Robin"};
    for (int i = 0; i < 4; i++) {
        printf("%-12s %-12.2f %-14.2f %-12.2f\n",
               names[i], metrics[i].waiting,
               metrics[i].turnaround, metrics[i].response);
    }

    if (save_csv(metrics, quantum, count)) {
        generate_plot_automatically();
    }
}
