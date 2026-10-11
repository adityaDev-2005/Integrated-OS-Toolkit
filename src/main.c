
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static int read_choice(void)
{
    char buffer[64];
    char *end;
    long value;

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 6;
    }

    errno = 0;
    value = strtol(buffer, &end, 10);

    if (end == buffer || errno != 0) {
        return -1;
    }

    while (*end == ' ' || *end == '\t' ||
           *end == '\n' || *end == '\r') {
        end++;
    }

    if (*end != '\0' || value < 1 || value > 6) {
        return -1;
    }

    return (int)value;
}

int main(void)
{
    int choice;

    while (1) {
        printf("\n");
        printf("========================================\n");
        printf("       INTEGRATED OS TOOLKIT\n");
        printf("========================================\n");
        printf("1. CPU Scheduling\n");
        printf("2. Page Replacement\n");
        printf("3. POSIX Process Demonstration\n");
        printf("4. Producer-Consumer Synchronization\n");
        printf("5. Run Benchmarks\n");
        printf("6. Exit\n");
        printf("----------------------------------------\n");
        printf("Enter your choice: ");
        fflush(stdout);

        choice = read_choice();

        switch (choice) {
            case 1:
                printf("\nCPU Scheduling selected.\n");
                printf("FCFS and Round Robin are our first targets.\n");
                break;

            case 2:
                printf("\nPage Replacement selected.\n");
                printf("FIFO and LRU are our first targets.\n");
                break;

            case 3:
                printf("\nPOSIX Process Demonstration selected.\n");
                break;

            case 4:
                printf("\nProducer-Consumer selected.\n");
                break;

            case 5:
                printf("\nBenchmarking selected.\n");
                break;

            case 6:
                printf("\nExiting Integrated OS Toolkit.\n");
                return 0;

            default:
                printf("\nInvalid input. Enter a number from 1 to 6.\n");
        }
    }
}
