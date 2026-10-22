#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

/*
 * Run the Phase E IPC and synchronization demonstration.
 *
 * The submenu offers a pthreads + POSIX semaphore bounded-buffer
 * producer-consumer and a fork() + pipe() example. Both use real threads,
 * semaphores and Linux system calls. Shared memory is excluded by scope
 * decision Q1, so no shared-memory mechanism is used here.
 */
void run_ipc_demo(void);

#endif
