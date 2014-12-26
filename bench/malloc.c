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
 * And then presumably do stats (in particular, mean and standard deviation) on
 * the output, and compare the results for different number of threads, amounts
 * of memory, etc. For example, with my r-fu wrapper around R:
 *
 *   for t in 1 2 4 8; do echo -n "$t "; r-fu msdevreduce out.$t; done > stat
 *   R
 *   > x=read.table("stat")
 *   > plot(x$V1,x$V2,type='h',log='y',lwd=2,bty='n',ylab="Memset (seconds)",xlab="Thread Count")
 *
 * This test is perhaps of interest on multiprocessor systems with large
 * amounts of memory, as for the test runs I've done performance will
 * improve, bottom out, and then worsen as the total number of "CPUs" on the
 * system is reached by the thread count.
 *
 * On OpenBSD, limits in login.conf(5) will doubtless need to be raised, and
 * other systems or hardware may set various limits, depending.
 */

#ifdef __linux__
#define _BSD_SOURCE
#define _GNU_SOURCE
#include <getopt.h>
#endif

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

#define MAX_THREADS 640UL

#define NSEC_IN_SEC 1000000000
#define USEC_IN_SEC 1000000

unsigned long Flag_Count;       // -c
unsigned long Flag_Memory;      // -m
unsigned long Flag_Threads;     // -t

size_t Mem_Size;

pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Job_Done = PTHREAD_COND_INITIALIZER;

const char *Program_Name;

void emit_help(void);
unsigned long flagtoul(const int flag, const char *flagarg,
                       const unsigned long min, const unsigned long max);
void *worker(void *unused);

int main(int argc, char *argv[])
{
    int ch;
    pthread_t *tids;
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
            setlinebuf(stdout);
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

    pthread_mutex_lock(&Lock);
    for (i = 0; i < Flag_Threads; i++) {
        if (pthread_create(&tids[i], NULL, worker, NULL) != 0)
            err(EX_OSERR, "could not pthread_create() thread %lu", i);
    }
    pthread_mutex_unlock(&Lock);

    for (;;) {
        pthread_mutex_lock(&Lock);
        if (Flag_Threads == 0)
            break;
        pthread_cond_wait(&Job_Done, &Lock);
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

unsigned long flagtoul(const int flag, const char *flagarg,
                       const unsigned long min, const unsigned long max)
{
    char *ep;
    unsigned long val;

    errno = 0;
    val = strtoul(optarg, &ep, 10);
    if (flagarg[0] == '\0' || *ep != '\0')
        errx(EX_DATAERR, "could not parse unsigned long from -%c '%s'", flag,
             flagarg);
    if (errno == ERANGE && val == ULONG_MAX)
        errx(EX_DATAERR, "value for -%c '%s' exceeds ULONG_MAX %lu", flag,
             flagarg, ULONG_MAX);
    if (min != 0 && val < min)
        errx(EX_DATAERR, "value for -%c '%s' is below min %lu", flag, flagarg,
             min);
    if (max != ULONG_MAX && val > max)
        errx(EX_DATAERR, "value for -%c '%s' exceeds max %lu", flag, flagarg,
             max);
    return val;
}

void *worker(void *unused)
{
    int *x;
    double delta_t;
    struct timeval before, after;
    //struct timespec before, after;
    unsigned long c;

    for (c = 0; c < Flag_Count; c++) {
        /* these two appear equivalent on OpenBSD, though clock_gettime does
         * offer alternate clocks... */
        gettimeofday(&before, NULL);
        //clock_gettime(CLOCK_REALTIME, &before);

        if ((x = malloc(Mem_Size)) == NULL)
            err(EX_OSERR, "could not malloc() list in thread");
        memset(x, 'y', Mem_Size);
        free(x);

        gettimeofday(&after, NULL);
        //clock_gettime(CLOCK_REALTIME, &after);

        /* hopefully close enough via double conversion; the alternative would
         * be to trade off timer resolution against the total time thereby
         * possible to measure, which I have not looked into. */
        delta_t =
            (after.tv_sec - before.tv_sec) + (after.tv_usec -
                                              before.tv_usec) /
            (double) USEC_IN_SEC;
        //delta_t = (after.tv_sec - before.tv_sec) + (after.tv_nsec - before.tv_nsec) / (double) NSEC_IN_SEC;
        printf("%.6f\n", delta_t);
    }

    pthread_mutex_lock(&Lock);
    Flag_Threads--;
    pthread_mutex_unlock(&Lock);
    pthread_cond_signal(&Job_Done);

    return (void *) 0;
}
