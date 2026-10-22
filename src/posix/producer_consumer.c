#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.h"
#include "producer_consumer.h"

#define PC_MIN_BUFFER 1
#define PC_MAX_BUFFER 100
#define PC_MIN_ITEMS 1
#define PC_MAX_ITEMS 10000
#define PC_MIN_THREADS 1
#define PC_MAX_THREADS 8

#define PIPE_MESSAGE "hello from child via pipe"
#define PIPE_MESSAGE_LENGTH (sizeof(PIPE_MESSAGE) - 1)
#define PIPE_READ_BUFFER 256

/*
 * Shared state of the bounded-buffer demonstration.
 *
 * `empty` counts free slots, `full` counts buffered items and `buffer_mutex`
 * protects the ring itself. `state_mutex` protects the bookkeeping counters,
 * so no thread ever holds `buffer_mutex` while taking `state_mutex` or blocks
 * on a semaphore.
 */
typedef struct {
    int *buffer;
    int capacity;
    int in;
    int out;
    int buffered;
    int max_buffered;
    pthread_mutex_t buffer_mutex;
    pthread_mutex_t state_mutex;
    sem_t empty;
    sem_t full;
    int next_item;
    int item_count;
    int remaining;
    int consumed_total;
    int duplicates;
    int invalid;
    int stop;
    unsigned char *seen;  /* one flag per item ID, so duplicates are detectable */
} PcContext;

typedef struct {
    PcContext *context;
    int index;
    int produced;
} ProducerArg;

typedef struct {
    PcContext *context;
    int index;
    int consumed;
} ConsumerArg;

static int sem_wait_retry(sem_t *semaphore)
{
    while (sem_wait(semaphore) == -1) {
        if (errno != EINTR)
            return -1;
    }

    return 0;
}

/*
 * Keep an abort wakeup flowing. A thread that observes `stop` after a
 * successful wait forwards one token, so a single post still releases every
 * thread blocked on the same semaphore.
 */
static void forward_token(sem_t *semaphore)
{
    if (sem_post(semaphore) == -1)
        fprintf(stderr, "[pc] sem_post: %s\n", strerror(errno));
}

static void report_sem_wait_failure(const char *role, int index,
                                    const char *name)
{
    fprintf(stderr, "[pc] %s %d: sem_wait (%s): %s\n",
            role, index, name, strerror(errno));
}

static void request_stop(PcContext *context)
{
    pthread_mutex_lock(&context->state_mutex);
    context->stop = 1;
    pthread_mutex_unlock(&context->state_mutex);
}

static int stopped(PcContext *context)
{
    int stopped_now;

    pthread_mutex_lock(&context->state_mutex);
    stopped_now = context->stop;
    pthread_mutex_unlock(&context->state_mutex);

    return stopped_now;
}

/* Release every thread that is blocked on a semaphore after a failure. */
static void wake_waiters(PcContext *context, int producers, int consumers)
{
    for (int i = 0; i < producers; i++)
        forward_token(&context->empty);

    for (int i = 0; i < consumers; i++)
        forward_token(&context->full);
}

static void restore_reservation(PcContext *context)
{
    pthread_mutex_lock(&context->state_mutex);
    context->remaining++;
    pthread_mutex_unlock(&context->state_mutex);
}

static void *producer_main(void *argument)
{
    ProducerArg *arg = argument;
    PcContext *context = arg->context;

    printf("[pc] producer %d: started\n", arg->index);

    for (;;) {
        int item = 0;
        int aborting;

        pthread_mutex_lock(&context->state_mutex);
        if (!context->stop && context->next_item <= context->item_count) {
            item = context->next_item;
            context->next_item++;
        }
        aborting = context->stop;
        pthread_mutex_unlock(&context->state_mutex);

        if (aborting || item == 0)
            break;

        if (sem_wait_retry(&context->empty) == -1) {
            report_sem_wait_failure("producer", arg->index, "empty");
            forward_token(&context->empty);
            break;
        }

        if (stopped(context)) {
            forward_token(&context->empty);
            break;
        }

        pthread_mutex_lock(&context->buffer_mutex);
        context->buffer[context->in] = item;
        context->in = (context->in + 1) % context->capacity;
        context->buffered++;

        if (context->buffered > context->max_buffered)
            context->max_buffered = context->buffered;

        pthread_mutex_unlock(&context->buffer_mutex);

        if (sem_post(&context->full) == -1) {
            fprintf(stderr, "[pc] producer %d: sem_post (full): %s\n",
                    arg->index, strerror(errno));
            break;
        }

        arg->produced++;
    }

    printf("[pc] producer %d: produced %d items\n", arg->index, arg->produced);
    return NULL;
}

static void *consumer_main(void *argument)
{
    ConsumerArg *arg = argument;
    PcContext *context = arg->context;

    printf("[pc] consumer %d: started\n", arg->index);

    for (;;) {
        int reserved = 0;
        int aborting;

        /*
         * Reserving one item before waiting keeps every consumer free of a
         * fixed quota: they all stop once the item count is exhausted.
         */
        pthread_mutex_lock(&context->state_mutex);
        if (!context->stop && context->remaining > 0) {
            context->remaining--;
            reserved = 1;
        }
        aborting = context->stop;
        pthread_mutex_unlock(&context->state_mutex);

        if (aborting) {
            if (reserved)
                restore_reservation(context);
            break;
        }

        if (!reserved)
            break;

        if (sem_wait_retry(&context->full) == -1) {
            report_sem_wait_failure("consumer", arg->index, "full");
            forward_token(&context->full);
            break;
        }

        if (stopped(context)) {
            forward_token(&context->full);
            break;
        }

        pthread_mutex_lock(&context->buffer_mutex);
        int item = context->buffer[context->out];
        context->out = (context->out + 1) % context->capacity;
        context->buffered--;
        pthread_mutex_unlock(&context->buffer_mutex);

        if (sem_post(&context->empty) == -1) {
            fprintf(stderr, "[pc] consumer %d: sem_post (empty): %s\n",
                    arg->index, strerror(errno));
            break;
        }

        pthread_mutex_lock(&context->state_mutex);
        if (item >= 1 && item <= context->item_count) {
            if (context->seen[item])
                context->duplicates++;
            else
                context->seen[item] = 1;
        } else {
            context->invalid++;
        }
        context->consumed_total++;
        pthread_mutex_unlock(&context->state_mutex);

        arg->consumed++;
    }

    printf("[pc] consumer %d: consumed %d items\n", arg->index, arg->consumed);
    return NULL;
}

static int release_context(PcContext *context, int buffer_mutex_ready,
                           int state_mutex_ready, int empty_ready,
                           int full_ready)
{
    int ok = 1;

    if (full_ready && sem_destroy(&context->full) == -1) {
        fprintf(stderr, "[pc] sem_destroy (full): %s\n", strerror(errno));
        ok = 0;
    }

    if (empty_ready && sem_destroy(&context->empty) == -1) {
        fprintf(stderr, "[pc] sem_destroy (empty): %s\n", strerror(errno));
        ok = 0;
    }

    if (state_mutex_ready && pthread_mutex_destroy(&context->state_mutex) != 0) {
        fprintf(stderr, "[pc] pthread_mutex_destroy (state) failed\n");
        ok = 0;
    }

    if (buffer_mutex_ready &&
        pthread_mutex_destroy(&context->buffer_mutex) != 0) {
        fprintf(stderr, "[pc] pthread_mutex_destroy (buffer) failed\n");
        ok = 0;
    }

    free(context->seen);
    free(context->buffer);
    context->seen = NULL;
    context->buffer = NULL;

    return ok;
}

static int run_producer_consumer(void)
{
    int capacity = 0;
    int item_count = 0;
    int producer_count = 0;
    int consumer_count = 0;

    printf("\n--- Producer-consumer: pthreads + POSIX semaphores ---\n");

    if (!read_integer("Bounded buffer size (1-100): ", PC_MIN_BUFFER,
                      PC_MAX_BUFFER, &capacity) ||
        !read_integer("Item count (1-10000): ", PC_MIN_ITEMS,
                      PC_MAX_ITEMS, &item_count) ||
        !read_integer("Producer threads (1-8): ", PC_MIN_THREADS,
                      PC_MAX_THREADS, &producer_count) ||
        !read_integer("Consumer threads (1-8): ", PC_MIN_THREADS,
                      PC_MAX_THREADS, &consumer_count)) {
        printf("\nInput ended before the demonstration was configured.\n");
        return 0;
    }

    printf("[pc] configuration: buffer=%d items=%d producers=%d consumers=%d\n",
           capacity, item_count, producer_count, consumer_count);

    PcContext context;

    memset(&context, 0, sizeof(context));
    context.capacity = capacity;
    context.item_count = item_count;
    context.next_item = 1;
    context.remaining = item_count;

    context.buffer = calloc((size_t)capacity, sizeof(*context.buffer));
    context.seen = calloc((size_t)item_count + 1, sizeof(*context.seen));

    if (context.buffer == NULL || context.seen == NULL) {
        fprintf(stderr, "[pc] allocation failed\n");
        free(context.seen);
        free(context.buffer);
        return 0;
    }

    int buffer_mutex_ready = 0;
    int state_mutex_ready = 0;
    int empty_ready = 0;
    int full_ready = 0;
    int ok = 1;
    int rc;

    rc = pthread_mutex_init(&context.buffer_mutex, NULL);
    if (rc != 0) {
        fprintf(stderr, "[pc] pthread_mutex_init (buffer): %s\n", strerror(rc));
        release_context(&context, 0, 0, 0, 0);
        return 0;
    }
    buffer_mutex_ready = 1;

    rc = pthread_mutex_init(&context.state_mutex, NULL);
    if (rc != 0) {
        fprintf(stderr, "[pc] pthread_mutex_init (state): %s\n", strerror(rc));
        release_context(&context, buffer_mutex_ready, 0, 0, 0);
        return 0;
    }
    state_mutex_ready = 1;

    if (sem_init(&context.empty, 0, (unsigned int)capacity) == -1) {
        fprintf(stderr, "[pc] sem_init (empty): %s\n", strerror(errno));
        release_context(&context, buffer_mutex_ready, state_mutex_ready, 0, 0);
        return 0;
    }
    empty_ready = 1;

    if (sem_init(&context.full, 0, 0) == -1) {
        fprintf(stderr, "[pc] sem_init (full): %s\n", strerror(errno));
        release_context(&context, buffer_mutex_ready, state_mutex_ready,
                        empty_ready, 0);
        return 0;
    }
    full_ready = 1;

    int empty_value = 0;
    int full_value = 0;

    if (sem_getvalue(&context.empty, &empty_value) == -1 ||
        sem_getvalue(&context.full, &full_value) == -1) {
        fprintf(stderr, "[pc] sem_getvalue: %s\n", strerror(errno));
        release_context(&context, buffer_mutex_ready, state_mutex_ready,
                        empty_ready, full_ready);
        return 0;
    }

    printf("[pc] semaphores: empty=%d full=%d (capacity enforced by semaphores)\n",
           empty_value, full_value);

    ProducerArg *producers = calloc((size_t)producer_count, sizeof(*producers));
    ConsumerArg *consumers = calloc((size_t)consumer_count, sizeof(*consumers));
    pthread_t *producer_threads = calloc((size_t)producer_count,
                                         sizeof(*producer_threads));
    pthread_t *consumer_threads = calloc((size_t)consumer_count,
                                         sizeof(*consumer_threads));

    if (producers == NULL || consumers == NULL ||
        producer_threads == NULL || consumer_threads == NULL) {
        fprintf(stderr, "[pc] allocation failed\n");
        free(consumer_threads);
        free(producer_threads);
        free(consumers);
        free(producers);
        release_context(&context, buffer_mutex_ready, state_mutex_ready,
                        empty_ready, full_ready);
        return 0;
    }

    printf("[pc] launching %d producer and %d consumer threads\n",
           producer_count, consumer_count);

    int created_producers = 0;
    int created_consumers = 0;
    int creation_failed = 0;

    /*
     * Consumers start first: they reserve work instead of receiving a fixed
     * quota, so the order only keeps the buffer from filling with no reader.
     */
    for (int index = 0; index < consumer_count; index++) {
        consumers[index].context = &context;
        consumers[index].index = index + 1;

        rc = pthread_create(&consumer_threads[index], NULL, consumer_main,
                            &consumers[index]);
        if (rc != 0) {
            fprintf(stderr, "[pc] pthread_create (consumer %d): %s\n",
                    index + 1, strerror(rc));
            creation_failed = 1;
            break;
        }

        created_consumers++;
    }

    if (!creation_failed) {
        for (int index = 0; index < producer_count; index++) {
            producers[index].context = &context;
            producers[index].index = index + 1;

            rc = pthread_create(&producer_threads[index], NULL, producer_main,
                                &producers[index]);
            if (rc != 0) {
                fprintf(stderr, "[pc] pthread_create (producer %d): %s\n",
                        index + 1, strerror(rc));
                creation_failed = 1;
                break;
            }

            created_producers++;
        }
    }

    if (creation_failed) {
        request_stop(&context);
        wake_waiters(&context, created_producers, created_consumers);
    }

    for (int index = 0; index < created_producers; index++) {
        rc = pthread_join(producer_threads[index], NULL);
        if (rc != 0) {
            fprintf(stderr, "[pc] pthread_join (producer %d): %s\n",
                    index + 1, strerror(rc));
            ok = 0;
        }
    }

    for (int index = 0; index < created_consumers; index++) {
        rc = pthread_join(consumer_threads[index], NULL);
        if (rc != 0) {
            fprintf(stderr, "[pc] pthread_join (consumer %d): %s\n",
                    index + 1, strerror(rc));
            ok = 0;
        }
    }

    int produced = 0;
    int consumed = 0;

    for (int index = 0; index < created_producers; index++)
        produced += producers[index].produced;

    for (int index = 0; index < created_consumers; index++)
        consumed += consumers[index].consumed;

    int missing = 0;

    for (int id = 1; id <= item_count; id++) {
        if (!context.seen[id])
            missing++;
    }

    int ring_empty = context.buffered == 0;

    if (created_producers != producer_count ||
        created_consumers != consumer_count)
        ok = 0;

    if (produced != item_count || consumed != item_count)
        ok = 0;

    if (context.duplicates != 0 || missing != 0 || context.invalid != 0)
        ok = 0;

    if (!ring_empty)
        ok = 0;

    if (context.max_buffered > capacity)
        ok = 0;

    printf("[pc] integrity: produced=%d consumed=%d duplicates=%d missing=%d "
           "invalid=%d buffer_empty=%s\n",
           produced, consumed, context.duplicates, missing, context.invalid,
           ring_empty ? "yes" : "no");
    printf("[pc] bounded buffer: peak occupancy %d of %d slots\n",
           context.max_buffered, capacity);

    free(consumer_threads);
    free(producer_threads);
    free(consumers);
    free(producers);

    if (!release_context(&context, buffer_mutex_ready, state_mutex_ready,
                         empty_ready, full_ready))
        ok = 0;

    printf("[pc] cleanup: threads joined, mutexes and semaphores destroyed\n");
    printf("[pc] bounded buffer: %s\n", ok ? "PASS" : "FAIL");

    return ok;
}

static int write_all(int descriptor, const char *data, size_t length)
{
    size_t written = 0;

    while (written < length) {
        ssize_t count = write(descriptor, data + written, length - written);

        if (count == -1) {
            if (errno == EINTR)
                continue;

            return -1;
        }

        written += (size_t)count;
    }

    return 0;
}

static void child_pipe_writer(int read_end, int write_end)
{
    /* _exit() skips stdio flushing, so the parent's buffers are not repeated. */
    if (close(read_end) == -1) {
        dprintf(STDERR_FILENO, "[pipe] child: close (read end): %s\n",
                strerror(errno));
        _exit(1);
    }

    dprintf(STDOUT_FILENO, "[pipe] child: closed unused read end\n");

    if (write_all(write_end, PIPE_MESSAGE, PIPE_MESSAGE_LENGTH) == -1) {
        dprintf(STDERR_FILENO, "[pipe] child: write: %s\n", strerror(errno));
        close(write_end);
        _exit(1);
    }

    dprintf(STDOUT_FILENO, "[pipe] child: wrote %d bytes\n",
            (int)PIPE_MESSAGE_LENGTH);

    if (close(write_end) == -1) {
        dprintf(STDERR_FILENO, "[pipe] child: close (write end): %s\n",
                strerror(errno));
        _exit(1);
    }

    dprintf(STDOUT_FILENO,
            "[pipe] child: closed write end (parent will see EOF)\n");
    _exit(0);
}

static int run_pipe_demo(void)
{
    int descriptors[2];

    printf("\n--- Pipe: fork + pipe ---\n");

    if (pipe(descriptors) == -1) {
        fprintf(stderr, "[pipe] pipe: %s\n", strerror(errno));
        return 0;
    }

    printf("[pipe] pipe created (read end and write end)\n");
    fflush(stdout);

    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "[pipe] fork: %s\n", strerror(errno));
        close(descriptors[0]);
        close(descriptors[1]);
        return 0;
    }

    if (pid == 0)
        child_pipe_writer(descriptors[0], descriptors[1]);

    int ok = 1;

    printf("[pipe] parent: child pid=%ld\n", (long)pid);

    if (close(descriptors[1]) == -1) {
        fprintf(stderr, "[pipe] parent: close (write end): %s\n",
                strerror(errno));
        ok = 0;
    } else {
        printf("[pipe] parent: closed unused write end\n");
    }

    char buffer[PIPE_READ_BUFFER];
    size_t received = 0;
    int eof = 0;

    for (;;) {
        ssize_t count = read(descriptors[0], buffer + received,
                             sizeof(buffer) - 1 - received);

        if (count == -1) {
            if (errno == EINTR)
                continue;

            fprintf(stderr, "[pipe] parent: read: %s\n", strerror(errno));
            ok = 0;
            break;
        }

        if (count == 0) {
            eof = 1;
            break;
        }

        received += (size_t)count;

        if (received == sizeof(buffer) - 1) {
            fprintf(stderr, "[pipe] parent: message exceeds the read buffer\n");
            ok = 0;
            break;
        }
    }

    buffer[received] = '\0';

    printf("[pipe] parent: read %d bytes\n", (int)received);
    printf("[pipe] parent: EOF observed: %s\n", eof ? "yes" : "no");
    printf("[pipe] parent: data = \"%s\"\n", buffer);

    if (!eof)
        ok = 0;

    if (received == PIPE_MESSAGE_LENGTH &&
        memcmp(buffer, PIPE_MESSAGE, PIPE_MESSAGE_LENGTH) == 0) {
        printf("[pipe] parent: data matches the message sent by the child\n");
    } else {
        printf("[pipe] parent: data does not match the message sent by the "
               "child\n");
        ok = 0;
    }

    if (close(descriptors[0]) == -1) {
        fprintf(stderr, "[pipe] parent: close (read end): %s\n",
                strerror(errno));
        ok = 0;
    } else {
        printf("[pipe] cleanup: both pipe ends closed\n");
    }

    int status = 0;
    pid_t reaped;

    do {
        reaped = waitpid(pid, &status, 0);
    } while (reaped == -1 && errno == EINTR);

    if (reaped == -1) {
        fprintf(stderr, "[pipe] waitpid: %s\n", strerror(errno));
        ok = 0;
    } else if (reaped != pid) {
        fprintf(stderr, "[pipe] waitpid: expected %ld, got %ld\n",
                (long)pid, (long)reaped);
        ok = 0;
    } else if (!WIFEXITED(status)) {
        if (WIFSIGNALED(status)) {
            fprintf(stderr, "[pipe] waitpid: child terminated by signal %d\n",
                    WTERMSIG(status));
        } else {
            fprintf(stderr, "[pipe] waitpid: child did not exit normally\n");
        }
        ok = 0;
    } else {
        printf("[pipe] waitpid: child exited with status %d\n",
               WEXITSTATUS(status));

        if (WEXITSTATUS(status) != 0)
            ok = 0;
    }

    printf("[pipe] pipe demo: %s\n", ok ? "PASS" : "FAIL");

    return ok;
}

void run_ipc_demo(void)
{
    int choice;

    printf("\n========== IPC AND SYNCHRONIZATION ==========\n");
    printf("Real threads, POSIX semaphores, processes and pipes; none simulated.\n");

    for (;;) {
        printf("\n1. Producer-Consumer (pthreads + POSIX semaphores)\n");
        printf("2. Pipe (fork + pipe)\n");
        printf("3. Back to main menu\n");

        if (!read_integer("Enter choice: ", 1, 3, &choice)) {
            printf("\n");
            return;
        }

        if (choice == 3)
            return;

        if (choice == 1)
            run_producer_consumer();
        else
            run_pipe_demo();
    }
}
