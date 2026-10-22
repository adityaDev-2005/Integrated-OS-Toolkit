#include <stdio.h>
#include <stdlib.h>

#include "benchmark.h"
#include "common.h"
#include "disk.h"

typedef struct {
    int min_cylinder;
    int max_cylinder;
    int head;
    int request_count;
    int requests[MAX_DISK_REQUESTS];
} DiskState;

typedef struct {
    int path[MAX_DISK_REQUESTS + 2];
    int path_length;
    int movement;
} DiskResult;

static void print_path(const DiskResult *result)
{
    printf("Head path: ");

    for (int i = 0; i < result->path_length; i++)
        printf("%s%d", i == 0 ? "" : " -> ", result->path[i]);

    printf("\n");
}

static void run_fcfs(const DiskState *state, DiskResult *result)
{
    int current = state->head;

    result->path[0] = current;
    result->path_length = 1;
    result->movement = 0;

    for (int i = 0; i < state->request_count; i++) {
        result->movement += abs(state->requests[i] - current);
        current = state->requests[i];
        result->path[result->path_length] = current;
        result->path_length++;
    }
}

/*
 * SSTF tie-break, fixed before implementation: the nearest cylinder is served
 * first; on equal distance the lower cylinder number wins; on equal cylinder
 * numbers the earlier request in the queue wins. The rule is deterministic, so
 * the reported order never depends on scan order beyond the queue itself.
 */
static void run_sstf(const DiskState *state, DiskResult *result)
{
    int served[MAX_DISK_REQUESTS] = {0};
    int current = state->head;

    result->path[0] = current;
    result->path_length = 1;
    result->movement = 0;

    for (int step = 0; step < state->request_count; step++) {
        int best = -1;
        int best_distance = 0;

        for (int i = 0; i < state->request_count; i++) {
            int distance;

            if (served[i])
                continue;

            distance = abs(state->requests[i] - current);

            if (best == -1 || distance < best_distance ||
                (distance == best_distance &&
                 state->requests[i] < state->requests[best])) {
                best = i;
                best_distance = distance;
            }
        }

        served[best] = 1;
        result->movement += best_distance;
        current = state->requests[best];
        result->path[result->path_length] = current;
        result->path_length++;
    }
}

/*
 * Conventional SCAN with a documented initial direction: the arm starts toward
 * the upper cylinder bound, services every pending request on the way up, and
 * always reaches the bound before reversing, so the run to the bound counts as
 * head movement even when the last request was served earlier. The reverse
 * sweep serves the remaining requests in descending order and ends at the last
 * one, because no work remains beyond it. This is SCAN, not LOOK: the LOOK
 * variant would reverse at the last request and report a shorter distance.
 */
static void run_scan(const DiskState *state, DiskResult *result)
{
    int served[MAX_DISK_REQUESTS] = {0};
    int current = state->head;

    result->path[0] = current;
    result->path_length = 1;
    result->movement = 0;

    for (;;) {
        int next = -1;

        for (int i = 0; i < state->request_count; i++) {
            if (served[i] || state->requests[i] < current)
                continue;

            if (next == -1 || state->requests[i] < state->requests[next])
                next = i;
        }

        if (next == -1)
            break;

        result->movement += state->requests[next] - current;
        current = state->requests[next];
        served[next] = 1;
        result->path[result->path_length] = current;
        result->path_length++;
    }

    if (current < state->max_cylinder) {
        result->movement += state->max_cylinder - current;
        current = state->max_cylinder;
        result->path[result->path_length] = current;
        result->path_length++;
    }

    for (;;) {
        int next = -1;

        for (int i = 0; i < state->request_count; i++) {
            if (served[i] || state->requests[i] > current)
                continue;

            if (next == -1 || state->requests[i] > state->requests[next])
                next = i;
        }

        if (next == -1)
            break;

        result->movement += current - state->requests[next];
        current = state->requests[next];
        served[next] = 1;
        result->path[result->path_length] = current;
        result->path_length++;
    }
}

static void report_policy(const char *name, const DiskResult *result)
{
    printf("\n---------- %s ----------\n", name);
    print_path(result);
    printf("Total head movement: %d cylinders\n", result->movement);
}

static void report_comparison(const DiskResult *fcfs, const DiskResult *sstf,
                              const DiskResult *scan)
{
    printf("\nComparison:\n");
    printf("%-8s%s\n", "Policy", "Total head movement");
    printf("%-8s%s\n", "------", "-------------------");
    printf("%-8s%d cylinders\n", "FCFS", fcfs->movement);
    printf("%-8s%d cylinders\n", "SSTF", sstf->movement);
    printf("%-8s%d cylinders\n", "SCAN", scan->movement);
}

static void print_requests(const DiskState *state)
{
    printf("Requests: ");

    for (int i = 0; i < state->request_count; i++)
        printf("%s%d", i == 0 ? "" : " ", state->requests[i]);

    printf("\n");
}

static void save_csv(const DiskState *state, const DiskResult *fcfs,
                     const DiskResult *sstf, const DiskResult *scan)
{
    static const char *const fields[] = {
        "algorithm", "total_head_movement", "request_count", "initial_head",
        "min_cylinder", "max_cylinder"
    };
    static const char *const names[] = {"FCFS", "SSTF", "SCAN"};
    const DiskResult *results[] = {fcfs, sstf, scan};
    BenchmarkCsv csv;

    if (!benchmark_csv_open(&csv, "disk_scheduling.csv",
                            "disk scheduling CSV")) {
        benchmark_report_csv_failure();
        return;
    }

    benchmark_csv_header(&csv, fields, sizeof(fields) / sizeof(fields[0]));

    for (int i = 0; i < 3; i++) {
        benchmark_csv_row_begin(&csv);
        benchmark_csv_text(&csv, names[i]);
        benchmark_csv_int(&csv, results[i]->movement);
        benchmark_csv_int(&csv, state->request_count);
        benchmark_csv_int(&csv, state->head);
        benchmark_csv_int(&csv, state->min_cylinder);
        benchmark_csv_int(&csv, state->max_cylinder);
        benchmark_csv_row_end(&csv);
    }

    if (!benchmark_csv_close(&csv)) {
        benchmark_report_csv_failure();
        return;
    }

    benchmark_report_csv_saved(benchmark_csv_relative(&csv));
    benchmark_generate_plot(
        "disk_scheduling",
        "data/results/disk_scheduling_comparison.png",
        "disk scheduling comparison graph");
}

void run_disk_scheduling(void)
{
    DiskState state;
    DiskResult fcfs;
    DiskResult sstf;
    DiskResult scan;
    char prompt[96];
    int request_count;

    printf("\nCylinder bounds describe the simulated disk surface.\n");

    if (!read_integer("Minimum cylinder (0-9998): ", 0,
                      MAX_DISK_CYLINDER - 1, &state.min_cylinder))
        return;

    snprintf(prompt, sizeof(prompt), "Maximum cylinder (%d-%d): ",
             state.min_cylinder + 1, MAX_DISK_CYLINDER);

    if (!read_integer(prompt, state.min_cylinder + 1, MAX_DISK_CYLINDER,
                      &state.max_cylinder))
        return;

    snprintf(prompt, sizeof(prompt), "Initial head position (%d-%d): ",
             state.min_cylinder, state.max_cylinder);

    if (!read_integer(prompt, state.min_cylinder, state.max_cylinder,
                      &state.head))
        return;

    snprintf(prompt, sizeof(prompt), "Number of requests (1-%d): ",
             MAX_DISK_REQUESTS);

    if (!read_integer(prompt, 1, MAX_DISK_REQUESTS, &request_count))
        return;

    state.request_count = request_count;

    for (int i = 0; i < request_count; i++) {
        snprintf(prompt, sizeof(prompt), "Request %d (%d-%d): ",
                 i + 1, state.min_cylinder, state.max_cylinder);

        if (!read_integer(prompt, state.min_cylinder, state.max_cylinder,
                          &state.requests[i]))
            return;
    }

    printf("\n========== DISK SCHEDULING ==========\n");
    printf("Cylinder range: %d-%d\n", state.min_cylinder, state.max_cylinder);
    printf("Initial head position: %d\n", state.head);
    print_requests(&state);

    run_fcfs(&state, &fcfs);
    run_sstf(&state, &sstf);
    run_scan(&state, &scan);

    report_policy("FCFS", &fcfs);
    report_policy("SSTF", &sstf);

    printf("\n---------- SCAN ----------\n");
    printf("Initial direction: increasing (toward cylinder %d)\n",
           state.max_cylinder);
    print_path(&scan);
    printf("Total head movement: %d cylinders\n", scan.movement);

    report_comparison(&fcfs, &sstf, &scan);

    printf("\nNotes:\n");
    printf("  SSTF ties go to the lower cylinder number, then to the earlier\n");
    printf("  request in the queue.\n");
    printf("  SCAN always reaches cylinder %d before reversing, so the run to\n",
           state.max_cylinder);
    printf("  the boundary is included in its total; this is SCAN, not LOOK.\n");

    save_csv(&state, &fcfs, &sstf, &scan);
}
