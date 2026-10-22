
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "banker.h"
#include "disk.h"
#include "file_alloc.h"
#include "memory.h"
#include "paging.h"
#include "producer_consumer.h"
#include "process_demo.h"
#include "scheduling.h"

static int read_choice(void)
{
    char buffer[64];
    char *end;
    long value;

    /* End of input exits, mirroring menu choice 10. */
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        return 10;

    errno = 0;
    value = strtol(buffer, &end, 10);

    if (end == buffer || errno != 0)
        return -1;

    while (*end == ' ' || *end == '\t' ||
           *end == '\n' || *end == '\r')
        end++;

    if (*end != '\0' || value < 1 || value > 10)
        return -1;

    return (int)value;
}

static void show_introduction(void)
{
    printf("\n");
    printf("====================================================\n");
    printf("          INTEGRATED OS TOOLKIT\n");
    printf("       ALGORITHMS AND BENCHMARKING SUITE\n");
    printf("====================================================\n");
    printf("Implemented modules:\n");
    printf("  1. CPU scheduling comparison (FCFS, SJF, Priority, RR)\n");
    printf("  2. Page replacement comparison (FIFO, LRU, LFU)\n");
    printf("  3. Memory allocation comparison (First, Best, Worst Fit)\n");
    printf("  4. POSIX process and file demonstration (real system calls)\n");
    printf("  5. IPC and synchronization demonstration (real threads and pipes)\n");
    printf("  6. Deadlock avoidance (Banker's algorithm)\n");
    printf("  7. Disk scheduling comparison (FCFS, SSTF, SCAN)\n");
    printf("  8. File allocation simulation (contiguous, linked, indexed)\n\n");
    printf("Use the menu to choose a module.\n");
    printf("====================================================\n");
}

static void show_about(void)
{
    printf("\nABOUT THE PROJECT\n");
    printf("-----------------\n");
    printf("Language: C\n");
    printf("Environment: Linux / POSIX\n");
    printf("Build system: GCC and Make\n");
    printf("Purpose: OS algorithms, POSIX demonstrations and benchmarking\n");
    printf("Implemented: CPU scheduling, page replacement, memory allocation,\n");
    printf("            deadlock avoidance, disk scheduling, file allocation,\n");
    printf("            the POSIX process/file demonstration, and the IPC\n");
    printf("            demonstration (pthreads, POSIX semaphores, pipes)\n");
    printf("Simulated: CPU scheduling, page replacement, memory allocation,\n");
    printf("           deadlock avoidance, disk scheduling and file allocation\n");
    printf("Real system calls: POSIX process/file demo, pthreads, semaphores\n");
    printf("                   and pipes\n");
    printf("Reporting: CSV and charts for CPU scheduling, page replacement,\n");
    printf("           memory allocation, disk scheduling and file allocation;\n");
    printf("           deadlock avoidance and POSIX/IPC remain console-only\n");
    printf("Benchmark metrics: simulated algorithm results, not runtime timings\n");
}

int main(void)
{
    int choice;

    show_introduction();

    while (1) {
        printf("\n========== MAIN MENU ==========\n");
        printf("1. Run CPU Scheduling Comparison\n");
        printf("2. Page Replacement Comparison\n");
        printf("3. Memory Allocation Comparison\n");
        printf("4. POSIX Process and File Demo\n");
        printf("5. IPC and Synchronization Demo\n");
        printf("6. Deadlock Avoidance (Banker's Algorithm)\n");
        printf("7. Disk Scheduling Comparison (FCFS, SSTF, SCAN)\n");
        printf("8. File Allocation Simulation\n");
        printf("9. About Project\n");
        printf("10. Exit\n");
        printf("Enter choice: ");
        fflush(stdout);

        choice = read_choice();

        switch (choice) {
            case 1:
                run_cpu_analyzer();
                break;
            case 2:
                run_page_replacement();
                break;
            case 3:
                run_memory_allocation();
                break;
            case 4:
                run_posix_demo();
                break;
            case 5:
                run_ipc_demo();
                break;
            case 6:
                run_deadlock_avoidance();
                break;
            case 7:
                run_disk_scheduling();
                break;
            case 8:
                run_file_allocation();
                break;
            case 9:
                show_about();
                break;
            case 10:
                printf("Exiting Integrated OS Toolkit.\n");
                return 0;
            default:
                printf("Invalid choice. Select 1 to 10.\n");
        }
    }
}
