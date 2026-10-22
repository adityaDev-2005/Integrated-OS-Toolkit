#ifndef FILE_ALLOC_H
#define FILE_ALLOC_H

/*
 * Simulated file allocation over a fixed set of disk blocks.
 *
 * The model is deliberately small and explicit:
 *   - The disk has 1..MAX_FILE_BLOCKS blocks of MIN_FILE_BLOCK_SIZE..
 *     MAX_FILE_BLOCK_SIZE bytes each. No real file or filesystem is touched.
 *   - File sizes are given in bytes; blocks needed = ceil(size / usable
 *     bytes per block).
 *   - Contiguous files occupy one run of data blocks.
 *   - Linked files reserve FILE_POINTER_BYTES in each data block for the next
 *     pointer, so only block_size - FILE_POINTER_BYTES bytes carry file data.
 *   - Indexed files use exactly one index block holding block_size /
 *     FILE_POINTER_BYTES pointers. A file that needs more data blocks than the
 *     index can address is rejected.
 */

#define MAX_FILE_BLOCKS 128
#define MAX_SIM_FILES 32
#define MAX_FILE_OPERATIONS 64
#define MAX_FILE_BYTES 1000000
#define MIN_FILE_BLOCK_SIZE 8
#define MAX_FILE_BLOCK_SIZE 4096
#define FILE_POINTER_BYTES 4

void run_file_allocation(void);

#endif
