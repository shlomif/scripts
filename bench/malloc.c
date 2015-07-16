/*
 * Do a lot of mallocs (and memsets, so should be a linear algorithm). Work is
 * done in the 'worker' function; the rest is parsing input and blocking main
 * until the threads get through said work. Optimization might nix the memset,
 * so compile perhaps via:
 *
 *   env CFLAGS="-g -std=c99 -Wall -pedantic -pipe -lpthread" make malloc
 *
 * And then try out various thread count (-t) and memory usage (-m) values over
 * a given number of test runs (-c):
 *
 *   for t in 1 2 4 8; do
 *     ./malloc -c 100 -m $((2**31)) -t $t > out.$t
 *   done
 *
 * NOTE that the -c count is per thread, so for equality between varied runs of
 * threads, -c will need to change proportionally to the thread count. The
 * downside of such is that low thread-count tests may take that much longer
 * to get through those extra tests.
 *
 * Then, presumably do stats (in particular, mean and standard deviation) on
 * the output, and compare the results for different number of threads, amounts
 * of memory, etc. For example, with my r-fu wrapper around R:
 *
 *   for t in 1 2 4 8; do echo -n "$t "; r-fu msdevreduce out.$t; done > stat
 *   R
 *   > x=read.table("stat")
 *   > plot(x$V1,x$V2,type='h',log='y',lwd=2,bty='n',ylab="Memset (seconds)",xlab="Thread Count")
 *
 * This test is perhaps of interest on multiprocessor systems with large
 * amounts of memory, as for the test runs I've done performance will improve,
 * bottom out, and then worsen as the total number of "CPUs" on the system is
 * reached by the thread count (assuming a large enough amount of memory is
 * consumed).
 *
 * On OpenBSD, limits in login.conf(5) will doubtless need to be raised, and
 * other systems or hardware may set various limits, depending.
 */

#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#define MAX_THREADS 640UL

#define NSEC_IN_SEC 1000000000

unsigned long Flag_Count;       // -c
unsigned long Flag_Memory;      // -m
unsigned long Flag_Threads;     // -t

unsigned long Threads_Completed;

size_t Mem_Size;

pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Job_Done = PTHREAD_COND_INITIALIZER;

const char *Program_Name;

void emit_help(void);
void *worker(void *unused);

int main(int argc, char *argv[])
{
    int ch;
    pthread_t *tids;
    struct timespec wait;
    unsigned long i;

#ifdef __OpenBSD__
    // since OpenBSD 5.4
    Program_Name = getprogname();
#else
    Program_Name = *argv;
#endif

    while ((ch = getopt(argc, argv, "c:h?lm:t:")) != -1) {
        switch (ch) {

        case 'c':
            Flag_Count = flagtoul(ch, optarg, 1UL, LONG_MAX);
            break;

        case 'l':
            setvbuf(stdout, (char *)NULL, _IOLBF, (size_t) 0);
            break;

        case 'm':
            Flag_Memory = flagtoul(ch, optarg, 1UL, LONG_MAX);
            break;

        case 't':
            Flag_Threads = flagtoul(ch, optarg, 1UL, MAX_THREADS);
            break;

        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (Flag_Count == 0 || Flag_Memory == 0 || Flag_Threads == 0)
        emit_help();

    if ((tids = calloc(sizeof(pthread_t), Flag_Threads)) == NULL)
        err(EX_OSERR, "could not calloc() threads list");

    Mem_Size = (size_t) Flag_Memory / Flag_Threads;

    for (i = 0; i < Flag_Threads; i++) {
        if (pthread_create(&tids[i], NULL, worker, NULL) != 0)
            err(EX_OSERR, "could not pthread_create() thread %lu", i);
    }

    wait.tv_sec = 7;
    wait.tv_nsec = 0;

    for (;;) {
        pthread_mutex_lock(&Lock);
        if (Threads_Completed == Flag_Threads)
            break;
        pthread_cond_timedwait(&Job_Done, &Lock, &wait);
        pthread_mutex_unlock(&Lock);
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    const char *shortname;
#ifdef __OpenBSD__
    shortname = Program_Name;
#else
    if ((shortname = strrchr(Program_Name, '/')) != NULL)
        shortname++;
    else
        shortname = Program_Name;
#endif

    fprintf(stderr, "Usage: %s [-l] -c count -m memory -t threads\n",
            shortname);

    exit(EX_USAGE);
}

void *worker(void *unused)
{
    int *x;
    long double delta_t;
    struct timespec before, after;
    unsigned long c;

    for (c = 0; c < Flag_Count; c++) {
        if (clock_gettime(CLOCK_REALTIME, &before) == -1)
            err(EX_OSERR, "clock_gettime() failed");

        if ((x = malloc(Mem_Size)) == NULL)
            err(EX_OSERR, "could not malloc() list in thread");
        memset(x, 'y', Mem_Size);
        free(x);

        if (clock_gettime(CLOCK_REALTIME, &after) == -1)
            err(EX_OSERR, "clock_gettime() failed");

        /* hopefully close enough via double conversion; the alternative would
         * be to trade off timer resolution against the total time thereby
         * possible to measure, which I have not looked into. */
        delta_t =
            (after.tv_sec - before.tv_sec) + (after.tv_nsec -
                                              before.tv_nsec) /
            (long double) NSEC_IN_SEC;
        printf("%.6Lf\n", delta_t);
    }

    pthread_mutex_lock(&Lock);
    Threads_Completed++;
    pthread_mutex_unlock(&Lock);
    pthread_cond_signal(&Job_Done);

    return (void *) 0;
}
