#include <stdio.h>
#include <string.h>

#include "benchmark.h"
#include "common.h"
#include "file_alloc.h"

enum {
    METHOD_CONTIGUOUS,
    METHOD_LINKED,
    METHOD_INDEXED
};

typedef struct {
    int type;       /* 1 = allocate, 2 = delete, 3 = retrieve */
    int file_id;
    int size_bytes; /* allocate operations only */
} FileOperation;

typedef struct {
    int allocated;
    int size_bytes;
    int data_blocks;
    int blocks[MAX_FILE_BLOCKS];
    int index_block; /* -1 when the method does not use an index block */
} FileRecord;

typedef struct {
    int block_count;
    int block_size;
    int used[MAX_FILE_BLOCKS];
    FileRecord files[MAX_SIM_FILES + 1];
    int alloc_ok;
    int alloc_failed;
    int delete_ok;
    int delete_failed;
    int retrieve_ok;
    int retrieve_failed;
} FileVolume;

typedef struct {
    int used_blocks;
    int active_files;
    int metadata_blocks;
    long requested_bytes;
    long allocated_bytes;
    int alloc_ok;
    int alloc_failed;
    int delete_ok;
    int delete_failed;
    int retrieve_ok;
    int retrieve_failed;
} FileSummary;

static const char *method_name(int method)
{
    switch (method) {
        case METHOD_CONTIGUOUS: return "Contiguous";
        case METHOD_LINKED:     return "Linked";
        case METHOD_INDEXED:    return "Indexed";
        default:                return "Unknown";
    }
}

static int index_capacity(int block_size)
{
    return block_size / FILE_POINTER_BYTES;
}

static int data_bytes_per_block(int method, int block_size)
{
    if (method == METHOD_LINKED)
        return block_size - FILE_POINTER_BYTES;

    return block_size;
}

static int blocks_for_size(int method, int size, int block_size)
{
    int per_block = data_bytes_per_block(method, block_size);

    return (size + per_block - 1) / per_block;
}

static int count_free(const FileVolume *volume)
{
    int free_blocks = 0;

    for (int block = 0; block < volume->block_count; block++) {
        if (!volume->used[block])
            free_blocks++;
    }

    return free_blocks;
}

static int first_free(const FileVolume *volume)
{
    for (int block = 0; block < volume->block_count; block++) {
        if (!volume->used[block])
            return block;
    }

    return -1;
}

static void print_blocks(const int *blocks, int count)
{
    printf("[");
    for (int i = 0; i < count; i++)
        printf("%s%d", i == 0 ? "" : " ", blocks[i]);
    printf("]");
}

static void print_volume_blocks(const FileVolume *volume, int used)
{
    int printed = 0;

    printf("[");
    for (int block = 0; block < volume->block_count; block++) {
        if (volume->used[block] == used)
            printf("%s%d", printed++ == 0 ? "" : " ", block);
    }
    printf("]");
}

static void init_volume(FileVolume *volume, int block_count, int block_size)
{
    memset(volume, 0, sizeof(*volume));
    volume->block_count = block_count;
    volume->block_size = block_size;

    for (int id = 0; id <= MAX_SIM_FILES; id++)
        volume->files[id].index_block = -1;
}

/*
 * Allocate one file. Every check runs before the disk is modified, so a failed
 * request never leaves a partial mapping behind.
 */
static void allocate_file(FileVolume *volume, int method, int file_id, int size)
{
    FileRecord *record = &volume->files[file_id];
    int data_blocks = blocks_for_size(method, size, volume->block_size);

    if (record->allocated) {
        printf("FAILED - file already allocated\n");
        volume->alloc_failed++;
        return;
    }

    if (method == METHOD_INDEXED) {
        int capacity = index_capacity(volume->block_size);
        int total_blocks = data_blocks + 1;
        int free_blocks = count_free(volume);
        int index_block;

        if (data_blocks > capacity) {
            printf("FAILED - needs %d data blocks, one index block holds %d\n",
                   data_blocks, capacity);
            volume->alloc_failed++;
            return;
        }

        if (free_blocks < total_blocks) {
            printf("FAILED - not enough free blocks (needs %d, %d free)\n",
                   total_blocks, free_blocks);
            volume->alloc_failed++;
            return;
        }

        index_block = first_free(volume);
        volume->used[index_block] = 1;

        memset(record, 0, sizeof(*record));
        record->allocated = 1;
        record->size_bytes = size;
        record->data_blocks = data_blocks;
        record->index_block = index_block;

        int found = 0;

        for (int block = 0;
             block < volume->block_count && found < data_blocks;
             block++) {
            if (!volume->used[block]) {
                volume->used[block] = 1;
                record->blocks[found] = block;
                found++;
            }
        }

        printf("success, index block %d, data blocks ", index_block);
        print_blocks(record->blocks, record->data_blocks);
        printf("\n");
        volume->alloc_ok++;
        return;
    }

    if (method == METHOD_CONTIGUOUS) {
        int start = -1;
        int run = 0;

        for (int block = 0; block < volume->block_count; block++) {
            if (!volume->used[block]) {
                run++;
                if (run == data_blocks) {
                    start = block - data_blocks + 1;
                    break;
                }
            } else {
                run = 0;
            }
        }

        if (start < 0) {
            printf("FAILED - no contiguous run of %d blocks (%d free)\n",
                   data_blocks, count_free(volume));
            volume->alloc_failed++;
            return;
        }

        for (int i = 0; i < data_blocks; i++) {
            volume->used[start + i] = 1;
            record->blocks[i] = start + i;
        }
    } else {
        int free_blocks = count_free(volume);
        int found = 0;

        if (free_blocks < data_blocks) {
            printf("FAILED - not enough free blocks (needs %d, %d free)\n",
                   data_blocks, free_blocks);
            volume->alloc_failed++;
            return;
        }

        for (int block = 0;
             block < volume->block_count && found < data_blocks;
             block++) {
            if (!volume->used[block]) {
                volume->used[block] = 1;
                record->blocks[found] = block;
                found++;
            }
        }
    }

    record->allocated = 1;
    record->size_bytes = size;
    record->data_blocks = data_blocks;
    record->index_block = -1;

    printf("success, data blocks ");
    print_blocks(record->blocks, record->data_blocks);
    printf("\n");
    volume->alloc_ok++;
}

static void delete_file(FileVolume *volume, int method, int file_id)
{
    FileRecord *record = &volume->files[file_id];

    if (!record->allocated) {
        printf("FAILED - file not allocated\n");
        volume->delete_failed++;
        return;
    }

    if (method == METHOD_INDEXED)
        printf("freed index block %d, data blocks ", record->index_block);
    else
        printf("freed data blocks ");

    print_blocks(record->blocks, record->data_blocks);
    printf("\n");

    for (int i = 0; i < record->data_blocks; i++)
        volume->used[record->blocks[i]] = 0;

    if (method == METHOD_INDEXED && record->index_block >= 0)
        volume->used[record->index_block] = 0;

    memset(record, 0, sizeof(*record));
    record->index_block = -1;
    volume->delete_ok++;
}

static void retrieve_file(FileVolume *volume, int method, int file_id)
{
    const FileRecord *record = &volume->files[file_id];

    if (!record->allocated) {
        printf("FAILED - file not allocated\n");
        volume->retrieve_failed++;
        return;
    }

    printf("size %d bytes, ", record->size_bytes);

    if (method == METHOD_INDEXED)
        printf("index block %d, ", record->index_block);

    printf("data blocks ");
    print_blocks(record->blocks, record->data_blocks);
    printf("\n");
    volume->retrieve_ok++;
}

static void summarize_volume(const FileVolume *volume, FileSummary *summary)
{
    memset(summary, 0, sizeof(*summary));

    for (int block = 0; block < volume->block_count; block++) {
        if (volume->used[block])
            summary->used_blocks++;
    }

    for (int id = 1; id <= MAX_SIM_FILES; id++) {
        const FileRecord *record = &volume->files[id];

        if (!record->allocated)
            continue;

        summary->active_files++;
        summary->requested_bytes += record->size_bytes;
        summary->allocated_bytes +=
            (long)record->data_blocks * volume->block_size;

        if (record->index_block >= 0) {
            summary->metadata_blocks++;
            summary->allocated_bytes += volume->block_size;
        }
    }

    summary->alloc_ok = volume->alloc_ok;
    summary->alloc_failed = volume->alloc_failed;
    summary->delete_ok = volume->delete_ok;
    summary->delete_failed = volume->delete_failed;
    summary->retrieve_ok = volume->retrieve_ok;
    summary->retrieve_failed = volume->retrieve_failed;
}

static void report_volume(const FileVolume *volume, const FileSummary *summary)
{
    printf("Block usage: used %d/%d; used blocks: ",
           summary->used_blocks, volume->block_count);
    print_volume_blocks(volume, 1);
    printf("; free blocks: ");
    print_volume_blocks(volume, 0);
    printf("\n");

    printf("Summary: files active %d; allocations %d ok / %d failed; "
           "deletions %d ok / %d failed; retrievals %d ok / %d failed\n",
           summary->active_files, summary->alloc_ok, summary->alloc_failed,
           summary->delete_ok, summary->delete_failed,
           summary->retrieve_ok, summary->retrieve_failed);

    printf("Space: requested %ld bytes, allocated %ld bytes, wasted %ld bytes; "
           "metadata blocks %d\n",
           summary->requested_bytes, summary->allocated_bytes,
           summary->allocated_bytes - summary->requested_bytes,
           summary->metadata_blocks);
}

static void run_volume(int method, const FileOperation *operations,
                       int operation_count, int block_count, int block_size,
                       FileSummary *summary)
{
    FileVolume volume;

    init_volume(&volume, block_count, block_size);
    printf("\n---------- %s allocation ----------\n", method_name(method));

    for (int i = 0; i < operation_count; i++) {
        const FileOperation *operation = &operations[i];

        printf("[op %d] ", i + 1);

        switch (operation->type) {
            case 1:
                printf("allocate file %d (%d bytes): ",
                       operation->file_id, operation->size_bytes);
                allocate_file(&volume, method,
                              operation->file_id, operation->size_bytes);
                break;
            case 2:
                printf("delete file %d: ", operation->file_id);
                delete_file(&volume, method, operation->file_id);
                break;
            default:
                printf("retrieve file %d: ", operation->file_id);
                retrieve_file(&volume, method, operation->file_id);
                break;
        }
    }

    summarize_volume(&volume, summary);
    report_volume(&volume, summary);
}

static void save_csv(const FileSummary summaries[], int block_count,
                     int block_size, int operation_count)
{
    static const char *const fields[] = {
        "algorithm", "used_blocks", "free_blocks", "active_files",
        "allocations_ok", "allocations_failed", "deletions_ok",
        "deletions_failed", "retrievals_ok", "retrievals_failed",
        "requested_bytes", "allocated_bytes", "wasted_bytes",
        "metadata_blocks", "block_count", "block_size", "operation_count"
    };
    static const char *const names[3] = {
        "Contiguous", "Linked", "Indexed"
    };
    BenchmarkCsv csv;

    if (!benchmark_csv_open(&csv, "file_allocation.csv",
                            "file allocation CSV")) {
        benchmark_report_csv_failure();
        return;
    }

    benchmark_csv_header(&csv, fields, sizeof(fields) / sizeof(fields[0]));

    for (int method = 0; method < 3; method++) {
        const FileSummary *summary = &summaries[method];

        benchmark_csv_row_begin(&csv);
        benchmark_csv_text(&csv, names[method]);
        benchmark_csv_int(&csv, summary->used_blocks);
        benchmark_csv_int(&csv, block_count - summary->used_blocks);
        benchmark_csv_int(&csv, summary->active_files);
        benchmark_csv_int(&csv, summary->alloc_ok);
        benchmark_csv_int(&csv, summary->alloc_failed);
        benchmark_csv_int(&csv, summary->delete_ok);
        benchmark_csv_int(&csv, summary->delete_failed);
        benchmark_csv_int(&csv, summary->retrieve_ok);
        benchmark_csv_int(&csv, summary->retrieve_failed);
        benchmark_csv_int(&csv, summary->requested_bytes);
        benchmark_csv_int(&csv, summary->allocated_bytes);
        benchmark_csv_int(&csv,
                          summary->allocated_bytes - summary->requested_bytes);
        benchmark_csv_int(&csv, summary->metadata_blocks);
        benchmark_csv_int(&csv, block_count);
        benchmark_csv_int(&csv, block_size);
        benchmark_csv_int(&csv, operation_count);
        benchmark_csv_row_end(&csv);
    }

    if (!benchmark_csv_close(&csv)) {
        benchmark_report_csv_failure();
        return;
    }

    benchmark_report_csv_saved(benchmark_csv_relative(&csv));
    benchmark_generate_plot(
        "file_allocation",
        "data/results/file_allocation_comparison.png",
        "file allocation comparison graph");
}

void run_file_allocation(void)
{
    FileOperation operations[MAX_FILE_OPERATIONS];
    FileSummary summaries[3];
    char prompt[128];
    int block_count;
    int block_size;
    int operation_count;

    printf("\nFile allocation simulation: all data stays in memory; "
           "no real filesystem is read or modified.\n");

    if (!read_integer("Simulated disk blocks (1-128): ",
                      1, MAX_FILE_BLOCKS, &block_count))
        return;

    snprintf(prompt, sizeof(prompt), "Block size in bytes (%d-%d): ",
             MIN_FILE_BLOCK_SIZE, MAX_FILE_BLOCK_SIZE);

    if (!read_integer(prompt, MIN_FILE_BLOCK_SIZE, MAX_FILE_BLOCK_SIZE,
                      &block_size))
        return;

    snprintf(prompt, sizeof(prompt), "Number of file operations (1-%d): ",
             MAX_FILE_OPERATIONS);

    if (!read_integer(prompt, 1, MAX_FILE_OPERATIONS, &operation_count))
        return;

    for (int i = 0; i < operation_count; i++) {
        snprintf(prompt, sizeof(prompt),
                 "Operation %d type (1=allocate, 2=delete, 3=retrieve): ",
                 i + 1);

        if (!read_integer(prompt, 1, 3, &operations[i].type))
            return;

        snprintf(prompt, sizeof(prompt), "File ID (1-%d): ", MAX_SIM_FILES);

        if (!read_integer(prompt, 1, MAX_SIM_FILES, &operations[i].file_id))
            return;

        operations[i].size_bytes = 0;

        if (operations[i].type == 1) {
            snprintf(prompt, sizeof(prompt),
                     "File size in bytes (1-%d): ", MAX_FILE_BYTES);

            if (!read_integer(prompt, 1, MAX_FILE_BYTES,
                              &operations[i].size_bytes))
                return;
        }
    }

    printf("\n========== FILE ALLOCATION ==========\n");
    printf("Simulated disk: %d blocks of %d bytes (%d bytes total), "
           "all blocks initially free\n",
           block_count, block_size, block_count * block_size);
    printf("Operations: %d\n", operation_count);
    printf("Simulation only: no real filesystem is read or modified.\n");

    run_volume(METHOD_CONTIGUOUS, operations, operation_count,
               block_count, block_size, &summaries[METHOD_CONTIGUOUS]);
    run_volume(METHOD_LINKED, operations, operation_count,
               block_count, block_size, &summaries[METHOD_LINKED]);
    run_volume(METHOD_INDEXED, operations, operation_count,
               block_count, block_size, &summaries[METHOD_INDEXED]);

    printf("\nNotes:\n");
    printf("  File size is in bytes; blocks needed = "
           "ceil(size / usable bytes per block).\n");
    printf("  Contiguous and indexed data blocks hold %d bytes each.\n",
           block_size);
    printf("  Linked blocks reserve %d bytes for the next pointer, so "
           "%d bytes per block are usable.\n",
           FILE_POINTER_BYTES, block_size - FILE_POINTER_BYTES);
    printf("  Indexed files use one index block holding %d pointers "
           "(%d bytes each); a file needing more data blocks is rejected.\n",
           index_capacity(block_size), FILE_POINTER_BYTES);
    printf("  Wasted space = allocated block bytes - requested bytes, so it "
           "includes last-block slack,\n");
    printf("  link-pointer slots and index blocks.\n");

    save_csv(summaries, block_count, block_size, operation_count);
}
