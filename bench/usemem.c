/*
 * (Ab)uses memory, perhaps for benchmarking or other such purposes, via
 * random reads and writes in a pool of memory with a specified number
 * of threads.
 *
 * On OpenBSD, limits in login.conf(5) will doubtless need to be raised,
 * and other systems or hardware may set various limits, depending.
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

#include "../montecarlo/aaarghs.h"

#define MAX_THREADS 640UL

#define MOSTMEMPOSSIBLE ( (ULONG_MAX < SIZE_MAX) ? ULONG_MAX : SIZE_MAX )

#define NSEC_IN_SEC 1000000000

unsigned long Flag_Memory;      // -m
unsigned long Flag_Threads;     // -t

/* Custom RNG as might be dealing with >32 bits of allocated memory, and
 * outside of OpenBSD system RNG functions generally sucking; these are
 * shared by the various threads, which may or may not be a problem. */
uint64_t jkiss_seedX;
uint64_t jkiss_seedY;
uint32_t jkiss_seedC1;
uint32_t jkiss_seedC2;
uint32_t jkiss_seedZ1;
uint32_t jkiss_seedZ2;

int *Memory;

pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Job_Done = PTHREAD_COND_INITIALIZER;

void emit_help(void);
uint64_t jkiss_rand64(void);
unsigned long jkiss_randof(const unsigned long max);
void *worker(void *unused);

int main(int argc, char *argv[])
{
    int ch;
    pthread_t *tids;
    struct timespec wait;
    unsigned long i;

    while ((ch = getopt(argc, argv, "h?m:t:")) != -1) {
        switch (ch) {

        case 'm':
            Flag_Memory = flagtoul(ch, optarg, 1UL, MOSTMEMPOSSIBLE);
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

    if (Flag_Memory == 0 || Flag_Threads == 0)
        emit_help();

    if ((tids = calloc(sizeof(pthread_t), Flag_Threads)) == NULL)
        err(EX_OSERR, "could not calloc() threads list");

    if ((Memory = calloc((size_t) Flag_Memory, sizeof(int))) == NULL)
        err(EX_OSERR, "could not calloc() %lu ints memory", Flag_Memory);

    for (i = 0; i < Flag_Threads; i++) {
        if (pthread_create(&tids[i], NULL, worker, NULL) != 0)
            err(EX_OSERR, "could not pthread_create() thread %lu", i);
    }

    wait.tv_sec = 7;
    wait.tv_nsec = 0;

    for (;;) {
        pthread_mutex_lock(&Lock);
        pthread_cond_timedwait(&Job_Done, &Lock, &wait);
        pthread_mutex_unlock(&Lock);
    }

    /* NOTREACHED */
    exit(1);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: usemem -m memory -t threads\n");
    exit(EX_USAGE);
}

// JLKISS64 RNG borrowed from public domain code in "Good Practice in (Pseudo)
// Random Number Generation for Bioinformatics Applications" by David Jones
// (Last revision May 7th 2010).
uint64_t jkiss_rand64(void)
{
    uint64_t t;
    jkiss_seedX = 1490024343005336237ULL * jkiss_seedX + 123456789;
    jkiss_seedY ^= jkiss_seedY << 21;
    jkiss_seedY ^= jkiss_seedY >> 17;
    jkiss_seedY ^= jkiss_seedY << 30;
    t = 4294584393ULL * jkiss_seedZ1 + jkiss_seedC1;
    jkiss_seedC1 = t >> 32;
    jkiss_seedZ1 = (uint32_t) t;
    t = 4246477509ULL * jkiss_seedZ2 + jkiss_seedC2;
    jkiss_seedC2 = t >> 32;
    jkiss_seedZ2 = (uint32_t) t;
    return jkiss_seedX + jkiss_seedY + jkiss_seedZ1 +
        ((uint64_t) jkiss_seedZ2 << 32);
}

unsigned long jkiss_randof(const unsigned long max)
{
    uint64_t r = jkiss_rand64();
    while (r > (uint64_t) max - 1) {
        r = jkiss_rand64();
    }
    return (unsigned long) r;
}

void *worker(void *unused)
{
    unsigned long idx;
    for (;;) {
        idx = jkiss_randof(Flag_Memory);
        Memory[idx] = (idx % 2 == 1)
            ? Memory[jkiss_randof(Flag_Memory)]
            : rand();
    }

    /* NOTREACHED */
}
