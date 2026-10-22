#include <stdio.h>

#include "benchmark.h"
#include "common.h"
#include "paging.h"

typedef struct {
    int page;       /* -1 while the frame is empty */
    int loaded_at;  /* reference index when the page was loaded */
    int last_used;  /* reference index of the most recent access */
    int uses;       /* accesses since the page was loaded */
} Frame;

typedef struct {
    int hits;
    int faults;
} PagingResult;

static void clear_frames(Frame frames[], int frame_count)
{
    for (int i = 0; i < frame_count; i++)
        frames[i] = (Frame){-1, -1, -1, 0};
}

static int find_page(const Frame frames[], int frame_count, int page)
{
    for (int i = 0; i < frame_count; i++) {
        if (frames[i].page == page)
            return i;
    }

    return -1;
}

static int find_empty(const Frame frames[], int frame_count)
{
    for (int i = 0; i < frame_count; i++) {
        if (frames[i].page == -1)
            return i;
    }

    return -1;
}

static int least_recently_used(const Frame frames[], int frame_count)
{
    int victim = 0;

    for (int i = 1; i < frame_count; i++) {
        if (frames[i].last_used < frames[victim].last_used)
            victim = i;
    }

    return victim;
}

/* LFU evicts the smallest access count; ties go to the earliest loaded page. */
static int least_frequently_used(const Frame frames[], int frame_count)
{
    int victim = 0;

    for (int i = 1; i < frame_count; i++) {
        if (frames[i].uses < frames[victim].uses ||
            (frames[i].uses == frames[victim].uses &&
             frames[i].loaded_at < frames[victim].loaded_at))
            victim = i;
    }

    return victim;
}

static PagingResult run_fifo(const int references[], int count,
                             int frame_count)
{
    Frame frames[MAX_FRAMES];
    PagingResult result = {0, 0};
    int next_victim = 0;

    clear_frames(frames, frame_count);

    for (int time = 0; time < count; time++) {
        int page = references[time];
        int slot = find_page(frames, frame_count, page);

        if (slot != -1) {
            result.hits++;
            continue;
        }

        result.faults++;
        slot = find_empty(frames, frame_count);

        if (slot == -1) {
            slot = next_victim;
            next_victim = (next_victim + 1) % frame_count;
        }

        frames[slot] = (Frame){page, time, time, 1};
    }

    return result;
}

static PagingResult run_lru(const int references[], int count,
                            int frame_count)
{
    Frame frames[MAX_FRAMES];
    PagingResult result = {0, 0};

    clear_frames(frames, frame_count);

    for (int time = 0; time < count; time++) {
        int page = references[time];
        int slot = find_page(frames, frame_count, page);

        if (slot != -1) {
            frames[slot].last_used = time;
            result.hits++;
            continue;
        }

        result.faults++;
        slot = find_empty(frames, frame_count);

        if (slot == -1)
            slot = least_recently_used(frames, frame_count);

        frames[slot] = (Frame){page, time, time, 1};
    }

    return result;
}

static PagingResult run_lfu(const int references[], int count,
                            int frame_count)
{
    Frame frames[MAX_FRAMES];
    PagingResult result = {0, 0};

    clear_frames(frames, frame_count);

    for (int time = 0; time < count; time++) {
        int page = references[time];
        int slot = find_page(frames, frame_count, page);

        if (slot != -1) {
            frames[slot].uses++;
            result.hits++;
            continue;
        }

        result.faults++;
        slot = find_empty(frames, frame_count);

        if (slot == -1)
            slot = least_frequently_used(frames, frame_count);

        frames[slot] = (Frame){page, time, time, 1};
    }

    return result;
}

static void save_csv(PagingResult fifo, PagingResult lru, PagingResult lfu,
                     int frame_count, int reference_count)
{
    static const char *const fields[] = {
        "algorithm", "hits", "faults", "frame_count", "reference_count"
    };
    static const char *const names[] = {"FIFO", "LRU", "LFU"};
    PagingResult results[] = {fifo, lru, lfu};
    BenchmarkCsv csv;

    if (!benchmark_csv_open(&csv, "page_replacement.csv",
                            "page replacement CSV")) {
        benchmark_report_csv_failure();
        return;
    }

    benchmark_csv_header(&csv, fields, sizeof(fields) / sizeof(fields[0]));

    for (int i = 0; i < 3; i++) {
        benchmark_csv_row_begin(&csv);
        benchmark_csv_text(&csv, names[i]);
        benchmark_csv_int(&csv, results[i].hits);
        benchmark_csv_int(&csv, results[i].faults);
        benchmark_csv_int(&csv, frame_count);
        benchmark_csv_int(&csv, reference_count);
        benchmark_csv_row_end(&csv);
    }

    if (!benchmark_csv_close(&csv)) {
        benchmark_report_csv_failure();
        return;
    }

    benchmark_report_csv_saved(benchmark_csv_relative(&csv));
    benchmark_generate_plot(
        "page_replacement",
        "data/results/page_replacement_comparison.png",
        "page replacement comparison graph");
}

void run_page_replacement(void)
{
    int references[MAX_PAGE_REFERENCES];
    int frame_count;
    int count;

    if (!read_integer("\nNumber of frames (1-100): ",
                      1, MAX_FRAMES, &frame_count))
        return;

    if (!read_integer("Number of page references (1-1000): ",
                      1, MAX_PAGE_REFERENCES, &count))
        return;

    printf("\nEnter %d page numbers (0-999).\n", count);

    for (int i = 0; i < count; i++) {
        char prompt[64];

        snprintf(prompt, sizeof(prompt), "Reference %d: ", i + 1);

        if (!read_integer(prompt, 0, 999, &references[i]))
            return;
    }

    PagingResult fifo = run_fifo(references, count, frame_count);
    PagingResult lru = run_lru(references, count, frame_count);
    PagingResult lfu = run_lfu(references, count, frame_count);

    printf("\n========== PAGE REPLACEMENT ==========\n");
    printf("Frames: %d   References: %d\n\n", frame_count, count);
    printf("%-8s%-6s%s\n", "Policy", "Hits", "Faults");
    printf("%-8s%-6d%d\n", "FIFO", fifo.hits, fifo.faults);
    printf("%-8s%-6d%d\n", "LRU", lru.hits, lru.faults);
    printf("%-8s%-6d%d\n", "LFU", lfu.hits, lfu.faults);
    printf("\nLFU tie-break: smallest access count, then earliest loaded page.\n");

    save_csv(fifo, lru, lfu, frame_count, count);
}
