#ifndef PROCESS_DEMO_H
#define PROCESS_DEMO_H

/*
 * Run the Phase D POSIX demonstration: fork/exec/wait/waitpid, process IDs,
 * file APIs, and directory APIs. All file writes stay in one temporary file
 * that is removed before the function returns.
 */
void run_posix_demo(void);

#endif
