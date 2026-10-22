#include <stdio.h>
#include <string.h>

#include "benchmark.h"
#include "common.h"
#include "memory.h"

enum {
    POLICY_FIRST_FIT,
    POLICY_BEST_FIT,
    POLICY_WORST_FIT,
    POLICY_COUNT
};

typedef struct {
    int block;      /* 1-based block index, 0 when the request failed */
    int requested;
    int allocated;  /* exact bytes granted, 0 when the request failed */
} RequestResult;

typedef struct {
    RequestResult requests[MAX_MEMORY_REQUESTS];
    int free_space[MAX_MEMORY_BLOCKS];
    int allocated_count;
    int failed_count;
    int internal_fragmentation;
    int largest_free_block;
} PolicyReport;

static int choose_block(const int free_space[], int block_count,
                        int request, int policy)
{
    int chosen = -1;

    for (int i = 0; i < block_count; i++) {
        if (free_space[i] < request)
            continue;

        if (policy == POLICY_FIRST_FIT)
            return i;

        if (chosen == -1)
            chosen = i;
        else if (policy == POLICY_BEST_FIT &&
                 free_space[i] < free_space[chosen])
            chosen = i;
        else if (policy == POLICY_WORST_FIT &&
                 free_space[i] > free_space[chosen])
            chosen = i;
    }

    return chosen;
}

static void run_policy(const int block_sizes[], int block_count,
                       const int request_sizes[], int request_count,
                       int policy, PolicyReport *report)
{
    memset(report, 0, sizeof(*report));

    for (int i = 0; i < block_count; i++)
        report->free_space[i] = block_sizes[i];

    for (int i = 0; i < request_count; i++) {
        int request = request_sizes[i];
        int slot = choose_block(report->free_space, block_count,
                                request, policy);
        RequestResult *result = &report->requests[i];

        result->requested = request;

        if (slot == -1) {
            report->failed_count++;
            continue;
        }

        /* Exact split: the whole request is granted, the rest stays free. */
        report->free_space[slot] -= request;
        result->block = slot + 1;
        result->allocated = request;
        report->allocated_count++;
        report->internal_fragmentation +=
            result->allocated - result->requested;
    }

    for (int i = 0; i < block_count; i++) {
        if (report->free_space[i] > report->largest_free_block)
            report->largest_free_block = report->free_space[i];
    }
}

static void describe_slot(int block, char *buffer, size_t size)
{
    if (block == 0)
        snprintf(buffer, size, "failed");
    else
        snprintf(buffer, size, "block %d", block);
}

static void print_requests(const int request_sizes[], int request_count,
                           const PolicyReport reports[])
{
    printf("%-9s%-9s%-15s%-15s%s\n",
           "Request", "Size", "First Fit", "Best Fit", "Worst Fit");

    for (int i = 0; i < request_count; i++) {
        char slots[POLICY_COUNT][16];

        for (int p = 0; p < POLICY_COUNT; p++)
            describe_slot(reports[p].requests[i].block,
                          slots[p], sizeof(slots[p]));

        printf("%-9d%-9d%-15s%-15s%s\n", i + 1, request_sizes[i],
               slots[0], slots[1], slots[2]);
    }
}

static void print_blocks(const int block_sizes[], int block_count,
                         const PolicyReport reports[])
{
    printf("\n%-7s%-9s%-16s%-16s%s\n",
           "Block", "Size", "First Fit free", "Best Fit free",
           "Worst Fit free");

    for (int i = 0; i < block_count; i++) {
        printf("%-7d%-9d%-16d%-16d%d\n", i + 1, block_sizes[i],
               reports[0].free_space[i],
               reports[1].free_space[i],
               reports[2].free_space[i]);
    }
}

static void print_summary(const PolicyReport reports[])
{
    static const char *const names[POLICY_COUNT] = {
        "First Fit", "Best Fit", "Worst Fit"
    };

    printf("\n%-12s%-11s%-8s%-16s%s\n",
           "Policy", "Allocated", "Failed", "Internal frag",
           "External frag (largest free block)");

    for (int p = 0; p < POLICY_COUNT; p++) {
        printf("%-12s%-11d%-8d%-16d%d\n", names[p],
               reports[p].allocated_count,
               reports[p].failed_count,
               reports[p].internal_fragmentation,
               reports[p].largest_free_block);
    }
}

static void save_csv(const PolicyReport reports[], int block_count,
                     int request_count)
{
    static const char *const fields[] = {
        "algorithm", "allocated_requests", "failed_requests",
        "internal_fragmentation", "largest_free_block", "block_count",
        "request_count"
    };
    static const char *const names[POLICY_COUNT] = {
        "First Fit", "Best Fit", "Worst Fit"
    };
    BenchmarkCsv csv;

    if (!benchmark_csv_open(&csv, "memory_allocation.csv",
                            "memory allocation CSV")) {
        benchmark_report_csv_failure();
        return;
    }

    benchmark_csv_header(&csv, fields, sizeof(fields) / sizeof(fields[0]));

    for (int p = 0; p < POLICY_COUNT; p++) {
        benchmark_csv_row_begin(&csv);
        benchmark_csv_text(&csv, names[p]);
        benchmark_csv_int(&csv, reports[p].allocated_count);
        benchmark_csv_int(&csv, reports[p].failed_count);
        benchmark_csv_int(&csv, reports[p].internal_fragmentation);
        benchmark_csv_int(&csv, reports[p].largest_free_block);
        benchmark_csv_int(&csv, block_count);
        benchmark_csv_int(&csv, request_count);
        benchmark_csv_row_end(&csv);
    }

    if (!benchmark_csv_close(&csv)) {
        benchmark_report_csv_failure();
        return;
    }

    benchmark_report_csv_saved(benchmark_csv_relative(&csv));
    benchmark_generate_plot(
        "memory_allocation",
        "data/results/memory_allocation_comparison.png",
        "memory allocation comparison graph");
}

void run_memory_allocation(void)
{
    int block_sizes[MAX_MEMORY_BLOCKS];
    int request_sizes[MAX_MEMORY_REQUESTS];
    int block_count;
    int request_count;
    PolicyReport reports[POLICY_COUNT];

    if (!read_integer("\nNumber of memory blocks (1-100): ",
                      1, MAX_MEMORY_BLOCKS, &block_count))
        return;

    for (int i = 0; i < block_count; i++) {
        char prompt[64];

        snprintf(prompt, sizeof(prompt),
                 "Size of block %d (1-1000000): ", i + 1);

        if (!read_integer(prompt, 1, MAX_MEMORY_SIZE, &block_sizes[i]))
            return;
    }

    if (!read_integer("Number of allocation requests (1-100): ",
                      1, MAX_MEMORY_REQUESTS, &request_count))
        return;

    for (int i = 0; i < request_count; i++) {
        char prompt[64];

        snprintf(prompt, sizeof(prompt),
                 "Size of request %d (1-1000000): ", i + 1);

        if (!read_integer(prompt, 1, MAX_MEMORY_SIZE, &request_sizes[i]))
            return;
    }

    for (int p = 0; p < POLICY_COUNT; p++)
        run_policy(block_sizes, block_count, request_sizes, request_count,
                   p, &reports[p]);

    printf("\n========== MEMORY ALLOCATION ==========\n");
    printf("Blocks: %d   Requests: %d\n\n", block_count, request_count);

    print_requests(request_sizes, request_count, reports);
    print_blocks(block_sizes, block_count, reports);
    print_summary(reports);

    save_csv(reports, block_count, request_count);
}
