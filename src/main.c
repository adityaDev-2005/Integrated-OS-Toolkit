
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "scheduling.h"

static int read_choice(void)
{
    char buffer[64];
    char *end;
    long value;

    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        return 7;

    errno = 0;
    value = strtol(buffer, &end, 10);

    if (end == buffer || errno != 0)
        return -1;

    while (*end == ' ' || *end == '\t' ||
           *end == '\n' || *end == '\r')
        end++;

    if (*end != '\0' || value < 1 || value > 7)
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
    printf("CPU SCHEDULING ANALYZER\n\n");
    printf("Compare four scheduling algorithms using one workload:\n");
    printf("  1. First Come First Served (FCFS)\n");
    printf("  2. Shortest Job First (non-preemptive)\n");
    printf("  3. Priority Scheduling (non-preemptive)\n");
    printf("  4. Round Robin\n\n");
    printf("Results include Gantt timelines, process metrics,\n");
    printf("average performance measures, and CSV export.\n");
    printf("====================================================\n");
}

static void show_about(void)
{
    printf("\nABOUT THE PROJECT\n");
    printf("-----------------\n");
    printf("Language: C\n");
    printf("Environment: Linux / POSIX\n");
    printf("Build system: GCC and Make\n");
    printf("Purpose: OS algorithm simulation and benchmarking\n");
    printf("Current implementation: CPU scheduling comparison\n");
    printf("Other modules: Under development\n");
}

int main(void)
{
    int choice;

    show_introduction();

    while (1) {
        printf("\n========== MAIN MENU ==========\n");
        printf("1. Run CPU Scheduling Comparison\n");
        printf("2. Page Replacement (planned)\n");
        printf("3. POSIX Process Demo (planned)\n");
        printf("4. Producer-Consumer Demo (planned)\n");
        printf("5. Benchmarking (planned)\n");
        printf("6. About Project\n");
        printf("7. Exit\n");
        printf("Enter choice: ");
        fflush(stdout);

        choice = read_choice();

        switch (choice) {
            case 1:
                run_cpu_analyzer();
                break;
            case 2:
            case 3:
            case 4:
            case 5:
                printf("\nThis module is planned for a later phase.\n");
                break;
            case 6:
                show_about();
                break;
            case 7:
                printf("Exiting Integrated OS Toolkit.\n");
                return 0;
            default:
                printf("Invalid choice. Select 1 to 7.\n");
        }
    }
}
