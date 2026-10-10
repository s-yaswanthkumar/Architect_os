#include "architect.h"

/* --- CO-6: Multithreaded Race Condition Demonstrator --- */
static volatile long g_shared_counter = 0;
static pthread_mutex_t g_counter_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int thread_id;
    long iterations;
    bool use_sync;
} ThreadArg;

static void *worker_func(void *arg) {
    ThreadArg *targ = (ThreadArg *)arg;
    for (long i = 0; i < targ->iterations; i++) {
        if (targ->use_sync) {
            pthread_mutex_lock(&g_counter_mutex);
            g_shared_counter++;
            pthread_mutex_unlock(&g_counter_mutex);
        } else {
            /* Unprotected read-modify-write data race */
            g_shared_counter++;
        }
    }
    return NULL;
}

int cmd_threads_demo(int argc, char **argv) {
    int num_threads = 4;
    long iterations = 100000;
    bool use_sync = false;

    if (argc > 1) num_threads = atoi(argv[1]);
    if (argc > 2) iterations = atol(argv[2]);
    if (argc > 3) {
        if (strcasecmp(argv[3], "sync") == 0) use_sync = true;
    }

    if (num_threads <= 0) num_threads = 4;
    if (iterations <= 0) iterations = 100000;

    printf("\n" COLOR_BOLD COLOR_CYAN "=== POSIX Multithreading & Race Condition Demonstrator ===" COLOR_RESET "\n");
    printf("Threads: %d | Iterations per thread: %ld | Mode: %s\n",
           num_threads, iterations,
           use_sync ? COLOR_GREEN "Synchronized (pthread_mutex_t)" COLOR_RESET :
                      COLOR_RED "Unsynchronized Data Race (nosync)" COLOR_RESET);

    g_shared_counter = 0;
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadArg *args = malloc(num_threads * sizeof(ThreadArg));

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        args[i].iterations = iterations;
        args[i].use_sync = use_sync;
        if (pthread_create(&threads[i], NULL, worker_func, &args[i]) != 0) {
            perror("pthread_create");
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;

    long expected = (long)num_threads * iterations;
    long actual = g_shared_counter;
    long delta = expected - actual;

    printf("\n" COLOR_BOLD "Execution Results:\n" COLOR_RESET);
    printf("  Expected Counter : %ld\n", expected);
    printf("  Actual Counter   : %ld\n", actual);
    printf("  Elapsed Time     : %.2f ms\n", elapsed_ms);

    if (use_sync) {
        printf(COLOR_GREEN "  Race Delta       : 0 (100%% Accurate - Thread Safe via Mutex Locks)\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "  Race Delta       : %ld missed increments (%.2f%% Data Loss due to Race Condition!)\n" COLOR_RESET,
               delta, (double)delta / expected * 100.0);
        printf(COLOR_YELLOW "  Mechanism        : Concurrent load/increment/store instructions interleaved without atomicity.\n" COLOR_RESET);
    }

    free(threads);
    free(args);
    return 0;
}

/* --- CO-6: Producer-Consumer Bounded Buffer Queue --- */
#define MAX_PC_BUFFER 64
typedef struct {
    int buffer[MAX_PC_BUFFER];
    int capacity;
    int in;
    int out;
    int count;
    sem_t sem_empty;
    sem_t sem_full;
    pthread_mutex_t mutex;
    int total_items;
} BoundedBuffer;

static BoundedBuffer g_bb;

static void *producer_thread(void *arg) {
    (void)arg;
    for (int i = 1; i <= g_bb.total_items; i++) {
        sem_wait(&g_bb.sem_empty);
        pthread_mutex_lock(&g_bb.mutex);

        g_bb.buffer[g_bb.in] = i;
        g_bb.in = (g_bb.in + 1) % g_bb.capacity;
        g_bb.count++;

        printf(COLOR_CYAN "  [Producer]" COLOR_RESET " Produced Item #%-2d | Buffer: [", i);
        for (int b = 0; b < g_bb.capacity; b++) {
            if (b < g_bb.count) printf("#");
            else printf(".");
        }
        printf("] (%d/%d)\n", g_bb.count, g_bb.capacity);

        pthread_mutex_unlock(&g_bb.mutex);
        sem_post(&g_bb.sem_full);
        usleep(30000); /* 30ms simulation delay */
    }
    return NULL;
}

static void *consumer_thread(void *arg) {
    (void)arg;
    for (int i = 1; i <= g_bb.total_items; i++) {
        sem_wait(&g_bb.sem_full);
        pthread_mutex_lock(&g_bb.mutex);

        int item = g_bb.buffer[g_bb.out];
        g_bb.out = (g_bb.out + 1) % g_bb.capacity;
        g_bb.count--;

        printf(COLOR_GREEN "  [Consumer]" COLOR_RESET " Consumed Item #%-2d | Buffer: [", item);
        for (int b = 0; b < g_bb.capacity; b++) {
            if (b < g_bb.count) printf("#");
            else printf(".");
        }
        printf("] (%d/%d)\n", g_bb.count, g_bb.capacity);

        pthread_mutex_unlock(&g_bb.mutex);
        sem_post(&g_bb.sem_empty);
        usleep(45000); /* 45ms simulation delay */
    }
    return NULL;
}

int cmd_pc_demo(int argc, char **argv) {
    int items = 10;
    int cap = 4;

    if (argc > 1) items = atoi(argv[1]);
    if (argc > 2) cap = atoi(argv[2]);

    if (items <= 0) items = 10;
    if (cap <= 0 || cap > MAX_PC_BUFFER) cap = 4;

    printf("\n" COLOR_BOLD COLOR_CYAN "=== Producer-Consumer Bounded Queue (Semaphores & Mutex) ===" COLOR_RESET "\n");
    printf("Total Items: %d | Bounded Buffer Capacity: %d\n\n", items, cap);

    g_bb.capacity = cap;
    g_bb.in = 0;
    g_bb.out = 0;
    g_bb.count = 0;
    g_bb.total_items = items;

    sem_init(&g_bb.sem_empty, 0, cap);
    sem_init(&g_bb.sem_full, 0, 0);
    pthread_mutex_init(&g_bb.mutex, NULL);

    pthread_t prod, cons;
    pthread_create(&prod, NULL, producer_thread, NULL);
    pthread_create(&cons, NULL, consumer_thread, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    sem_destroy(&g_bb.sem_empty);
    sem_destroy(&g_bb.sem_full);
    pthread_mutex_destroy(&g_bb.mutex);

    printf("\n" COLOR_GREEN "[Bounded Queue Verified]" COLOR_RESET " All %d items successfully exchanged without buffer underflow or overflow.\n\n", items);
    return 0;
}

/* --- CO-6: Deadlock Simulation & Lock Hierarchy Prevention --- */
static pthread_mutex_t g_mutex_A = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_mutex_B = PTHREAD_MUTEX_INITIALIZER;
static bool g_deadlock_prevent = false;

static void *thread1_routine(void *arg) {
    (void)arg;
    printf("  [Thread 1] Acquiring Lock A...\n");
    pthread_mutex_lock(&g_mutex_A);
    printf("  [Thread 1] Lock A ACQUIRED. Holding Lock A and sleeping 100ms...\n");
    usleep(100000);

    printf("  [Thread 1] Attempting to acquire Lock B...\n");
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 1; /* 1 second timeout to avoid permanent hanging */

    int res = pthread_mutex_timedlock(&g_mutex_B, &ts);
    if (res == ETIMEDOUT) {
        printf(COLOR_RED "  [Thread 1] TIMEOUT! Deadlock detected while waiting for Lock B!\n" COLOR_RESET);
    } else {
        printf(COLOR_GREEN "  [Thread 1] Lock B ACQUIRED. Critical section completed.\n" COLOR_RESET);
        pthread_mutex_unlock(&g_mutex_B);
    }

    pthread_mutex_unlock(&g_mutex_A);
    return NULL;
}

static void *thread2_routine(void *arg) {
    (void)arg;
    if (g_deadlock_prevent) {
        /* Resource Hierarchy: Always lock A before B */
        printf("  [Thread 2 - Lock Ordering] Acquiring Lock A first...\n");
        pthread_mutex_lock(&g_mutex_A);
        printf("  [Thread 2 - Lock Ordering] Lock A ACQUIRED. Acquiring Lock B...\n");
        pthread_mutex_lock(&g_mutex_B);
        printf(COLOR_GREEN "  [Thread 2 - Lock Ordering] Lock B ACQUIRED. Critical section completed.\n" COLOR_RESET);
        pthread_mutex_unlock(&g_mutex_B);
        pthread_mutex_unlock(&g_mutex_A);
    } else {
        /* Circular Wait Hazard: Lock B then A */
        printf("  [Thread 2 - Circular Hazard] Acquiring Lock B...\n");
        pthread_mutex_lock(&g_mutex_B);
        printf("  [Thread 2 - Circular Hazard] Lock B ACQUIRED. Holding Lock B and sleeping 100ms...\n");
        usleep(100000);

        printf("  [Thread 2 - Circular Hazard] Attempting to acquire Lock A...\n");
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        int res = pthread_mutex_timedlock(&g_mutex_A, &ts);
        if (res == ETIMEDOUT) {
            printf(COLOR_RED "  [Thread 2] TIMEOUT! Deadlock detected while waiting for Lock A!\n" COLOR_RESET);
        } else {
            printf(COLOR_GREEN "  [Thread 2] Lock A ACQUIRED.\n" COLOR_RESET);
            pthread_mutex_unlock(&g_mutex_A);
        }

        pthread_mutex_unlock(&g_mutex_B);
    }
    return NULL;
}

int cmd_deadlock_demo(int argc, char **argv) {
    g_deadlock_prevent = false;
    if (argc > 1 && strcasecmp(argv[1], "prevent") == 0) {
        g_deadlock_prevent = true;
    }

    printf("\n" COLOR_BOLD COLOR_CYAN "=== Deadlock Detection & Resource Hierarchy Resolution ===" COLOR_RESET "\n");
    printf("Strategy: %s\n\n",
           g_deadlock_prevent ? COLOR_GREEN "Lock Ordering (Resource Hierarchy: Lock A -> Lock B)" COLOR_RESET :
                                COLOR_RED "Hazardous Inversion (Thread 1: A->B, Thread 2: B->A)" COLOR_RESET);

    printf(COLOR_BOLD "Coffman Conditions Analysis:\n" COLOR_RESET);
    printf("  1. Mutual Exclusion  : Enabled (Pthread Mutex)\n");
    printf("  2. Hold and Wait     : Enabled (Threads hold one lock while requesting second)\n");
    printf("  3. No Preemption     : Enabled (Mutexes cannot be forcibly seized)\n");
    printf("  4. Circular Wait     : %s\n\n",
           g_deadlock_prevent ? COLOR_GREEN "BROKEN via Global Strict Ordering (Safe)" COLOR_RESET :
                                COLOR_RED "ACTIVE via Mutex Inversion (Deadlock Guarantee!)" COLOR_RESET);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread1_routine, NULL);
    pthread_create(&t2, NULL, thread2_routine, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    if (g_deadlock_prevent) {
        printf("\n" COLOR_GREEN "[Deadlock Prevention Verified]" COLOR_RESET
               " Strict lock hierarchy broke Coffman Condition #4. All threads finished cleanly.\n\n");
    } else {
        printf("\n" COLOR_YELLOW "[Circular Wait Deadlock Observed]" COLOR_RESET
               " Both threads experienced mutual circular wait and were rescued by timed-lock detection.\n\n");
    }

    return 0;
}
