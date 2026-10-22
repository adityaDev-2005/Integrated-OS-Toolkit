#include <stdio.h>

#include "banker.h"
#include "common.h"

/*
 * Banker's algorithm is a simulation of a resource-allocation state. It reads
 * the allocation, maximum and available values, derives the need matrix and
 * runs the safety algorithm. No real kernel resource table is involved.
 */

typedef struct {
    int process_count;
    int resource_count;
    int allocation[MAX_BANKER_PROCESSES][MAX_BANKER_RESOURCES];
    int maximum[MAX_BANKER_PROCESSES][MAX_BANKER_RESOURCES];
    int need[MAX_BANKER_PROCESSES][MAX_BANKER_RESOURCES];
    int available[MAX_BANKER_RESOURCES];
} BankerState;

typedef struct {
    int safe;
    int sequence[MAX_BANKER_PROCESSES];
    int sequence_length;
    int unfinished[MAX_BANKER_PROCESSES];
    int unfinished_count;
} SafetyResult;

static void print_vector(const int values[], int count)
{
    printf("(");

    for (int i = 0; i < count; i++)
        printf("%s%d", i == 0 ? "" : " ", values[i]);

    printf(")");
}

/*
 * C11 does not convert int (*)[N] to const int (*)[N], so the matrix is taken
 * without const even though this function only reads it.
 */
static void print_matrix(const char *title,
                         int matrix[MAX_BANKER_PROCESSES][MAX_BANKER_RESOURCES],
                         int process_count, int resource_count)
{
    printf("\n%s\n", title);
    printf("%-6s", "");

    for (int j = 0; j < resource_count; j++) {
        if (j + 1 == resource_count)
            printf("R%d", j + 1);
        else
            printf("R%-4d", j + 1);
    }

    printf("\n");

    for (int i = 0; i < process_count; i++) {
        printf("P%-5d", i + 1);

        for (int j = 0; j < resource_count; j++) {
            if (j + 1 == resource_count)
                printf("%d", matrix[i][j]);
            else
                printf("%-5d", matrix[i][j]);
        }

        printf("\n");
    }
}

static int read_matrix(BankerState *state, const char *label,
                       int matrix[MAX_BANKER_PROCESSES]
                                 [MAX_BANKER_RESOURCES])
{
    for (int i = 0; i < state->process_count; i++) {
        for (int j = 0; j < state->resource_count; j++) {
            char prompt[64];

            snprintf(prompt, sizeof(prompt), "%s P%d R%d (0-%d): ",
                     label, i + 1, j + 1, MAX_BANKER_VALUE);

            if (!read_integer(prompt, 0, MAX_BANKER_VALUE, &matrix[i][j]))
                return 0;
        }
    }

    return 1;
}

/*
 * An allocation entry larger than the same process's maximum claim is
 * inconsistent, so the state is rejected instead of being analyzed. Values
 * that are merely out of range (negative or malformed) are already rejected
 * and re-prompted by read_integer().
 */
static int validate_state(const BankerState *state)
{
    for (int i = 0; i < state->process_count; i++) {
        for (int j = 0; j < state->resource_count; j++) {
            if (state->allocation[i][j] > state->maximum[i][j]) {
                printf("\n[banker] invalid state: Allocation P%d R%d = %d "
                       "exceeds Maximum P%d R%d = %d\n",
                       i + 1, j + 1, state->allocation[i][j],
                       i + 1, j + 1, state->maximum[i][j]);
                printf("[banker] state: REJECTED\n");
                printf("[banker] note: an allocation cannot exceed the maximum "
                       "claim of the same process\n");
                return 0;
            }
        }
    }

    return 1;
}

static void compute_need(BankerState *state)
{
    for (int i = 0; i < state->process_count; i++) {
        for (int j = 0; j < state->resource_count; j++)
            state->need[i][j] = state->maximum[i][j] - state->allocation[i][j];
    }
}

static int can_finish(const BankerState *state, int process,
                      const int work[])
{
    for (int j = 0; j < state->resource_count; j++) {
        if (state->need[process][j] > work[j])
            return 0;
    }

    return 1;
}

/*
 * The lowest-numbered ready process is chosen first, so the reported sequence
 * is deterministic when several valid sequences exist.
 */
static SafetyResult run_safety(const BankerState *state)
{
    SafetyResult result = {0};
    int work[MAX_BANKER_RESOURCES];
    int finished[MAX_BANKER_PROCESSES] = {0};
    int step = 0;

    for (int j = 0; j < state->resource_count; j++)
        work[j] = state->available[j];

    printf("\nSafety analysis (work starts as available):\n");
    printf("  work ");
    print_vector(work, state->resource_count);
    printf("\n");

    for (;;) {
        int selected = -1;

        for (int i = 0; i < state->process_count; i++) {
            if (!finished[i] && can_finish(state, i, work)) {
                selected = i;
                break;
            }
        }

        if (selected == -1)
            break;

        printf("  step %d: P%d can finish (need ", step + 1, selected + 1);
        print_vector(state->need[selected], state->resource_count);
        printf(" <= work ");
        print_vector(work, state->resource_count);
        printf("); release ");
        print_vector(state->allocation[selected], state->resource_count);

        for (int j = 0; j < state->resource_count; j++)
            work[j] += state->allocation[selected][j];

        printf(" -> work ");
        print_vector(work, state->resource_count);
        printf("\n");

        finished[selected] = 1;
        result.sequence[result.sequence_length] = selected;
        result.sequence_length++;
        step++;
    }

    result.safe = (result.sequence_length == state->process_count);

    if (!result.safe) {
        printf("  no remaining process can finish with the current work\n");

        for (int i = 0; i < state->process_count; i++) {
            if (!finished[i]) {
                result.unfinished[result.unfinished_count] = i;
                result.unfinished_count++;
            }
        }
    }

    return result;
}

static void report_result(const BankerState *state, const SafetyResult *result)
{
    printf("\n");

    if (result->safe) {
        printf("[banker] state: SAFE\n");
        printf("[banker] safe sequence: ");

        for (int i = 0; i < result->sequence_length; i++)
            printf("%sP%d", i == 0 ? "" : " -> ", result->sequence[i] + 1);

        printf("\n");
        printf("[banker] all %d processes can finish in the reported order\n",
               state->process_count);
        printf("[banker] tie-break: the lowest-numbered ready process is chosen "
               "first, so\n");
        printf("        one valid sequence is reported when several exist\n");
    } else {
        printf("[banker] state: UNSAFE\n");
        printf("[banker] unfinished processes: ");

        for (int i = 0; i < result->unfinished_count; i++)
            printf("%sP%d", i == 0 ? "" : ", ", result->unfinished[i] + 1);

        printf("\n");
        printf("[banker] note: unsafe is not the same as deadlocked; it means no "
               "safe\n");
        printf("        completion order is guaranteed for this state\n");
    }
}

void run_deadlock_avoidance(void)
{
    BankerState state = {0};
    SafetyResult result;
    int process_count;
    int resource_count;

    if (!read_integer("\nNumber of processes (1-20): ", 1,
                      MAX_BANKER_PROCESSES, &process_count))
        return;

    if (!read_integer("Number of resource types (1-20): ", 1,
                      MAX_BANKER_RESOURCES, &resource_count))
        return;

    state.process_count = process_count;
    state.resource_count = resource_count;

    printf("\nEnter the Allocation matrix (%d processes, %d resource types).\n",
           process_count, resource_count);

    if (!read_matrix(&state, "Allocation", state.allocation))
        return;

    printf("\nEnter the Maximum matrix (%d processes, %d resource types).\n",
           process_count, resource_count);

    if (!read_matrix(&state, "Maximum", state.maximum))
        return;

    printf("\nEnter the Available vector (%d values).\n", resource_count);

    for (int j = 0; j < resource_count; j++) {
        char prompt[64];

        snprintf(prompt, sizeof(prompt), "Available R%d (0-%d): ",
                 j + 1, MAX_BANKER_VALUE);

        if (!read_integer(prompt, 0, MAX_BANKER_VALUE, &state.available[j]))
            return;
    }

    printf("\n========== DEADLOCK AVOIDANCE ==========\n");
    printf("Banker's algorithm with %d processes and %d resource types.\n",
           process_count, resource_count);

    print_matrix("Allocation matrix:", state.allocation,
                 process_count, resource_count);
    print_matrix("Maximum matrix:", state.maximum,
                 process_count, resource_count);

    if (!validate_state(&state))
        return;

    compute_need(&state);

    print_matrix("Need matrix (Maximum - Allocation):", state.need,
                 process_count, resource_count);

    printf("\nAvailable vector: ");
    print_vector(state.available, resource_count);
    printf("\n");

    result = run_safety(&state);
    report_result(&state, &result);
}
