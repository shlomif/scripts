/*
 * (Ab)uses memory, perhaps for benchmarking or other such purposes, via
 * random reads and writes in a pool of memory with a specified number
 * of threads.
 *
 * On OpenBSD, limits in login.conf(5) will doubtless need to be raised,
 * and other systems or hardware may set various limits, depending, or
 * launch multiple instances of this program, e.g. via
 *
 *   for i in ...; do ./usemem -M -t 8 & done
 *
 * To let get process get the most memory possible (the Linux OOM killer
 * will step in, though) and with a thread count suitable to the number
 * of CPUs, or whatever.
 *
 * The original intent of this code was to try to trigger a race condition:
 *
 *   alarm(...);
 *   somethingthatblocks();
 *   alarm(0);
 *     (paraphrased from Stevens, APUE (1st edition), p. 286.
 *
 * whereby the system is made busy enough that the SIGALRM happens before
 * the somethingthatblocks() call can be started, but after the alarm(...)
 * is established. It is far more likely that the system will be rendered
 * unusable (and monitoring notice this) than the system hit this unlikely
 * edge case; if a system has been inordinately busy or slow, a reboot might
 * be in order anyways, which would certainly clear anything thus stuck (and
 * unmonitored).
 */

#ifdef __linux__
#define _BSD_SOURCE
#define _GNU_SOURCE
#include <fcntl.h>
#include <getopt.h>
#endif

#ifdef __DARWIN__
#include <fcntl.h>
#endif

#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#include "../montecarlo/aaarghs.h"

#define MAX_THREADS 640UL

#define MOSTMEMPOSSIBLE ( (ULONG_MAX < SIZE_MAX) ? ULONG_MAX : SIZE_MAX )

bool Flag_Auto_Mem;             // -M
unsigned long Flag_Memory;      // -m
unsigned long Flag_Threads;     // -t

/* Custom RNG as might be dealing with >32 bits of allocated memory,
 * and outside of OpenBSD system RNG functions generally sucking; these
 * vars are shared by the various threads, which may or may not be a
 * problem. (The randomness is to hopefully thwart any CPU cache
 * prediction algos.) */
uint64_t jkiss_seedX;
uint64_t jkiss_seedY;
uint32_t jkiss_seedC1 = 6543217;
uint32_t jkiss_seedC2 = 1732654;
uint32_t jkiss_seedZ1 = 43219876;
uint32_t jkiss_seedZ2 = 21987643;

int *Memory;

pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Job_Done = PTHREAD_COND_INITIALIZER;

void emit_help(void);
void jkiss_init(void);
uint64_t jkiss_rand64(void);
unsigned long jkiss_randof(const unsigned long max);
void *worker(void *unused);

int main(int argc, char *argv[])
{
    int ch;
    pthread_t *tids;
    struct timespec wait;
    unsigned long i;

    while ((ch = getopt(argc, argv, "h?Mm:t:")) != -1) {
        switch (ch) {

        case 'M':
            Flag_Auto_Mem = true;
            break;

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

    if ((Flag_Memory == 0 && !Flag_Auto_Mem) || Flag_Threads == 0)
        emit_help();

    jkiss_init();

    if ((tids = calloc(sizeof(pthread_t), Flag_Threads)) == NULL)
        err(EX_OSERR, "could not calloc() threads list");

    if (Flag_Auto_Mem) {
        bool gotmem = false;
        Flag_Memory = SIZE_MAX;
        while (Flag_Memory) {
            if ((Memory = calloc((size_t) Flag_Memory, sizeof(int))) == NULL) {
                Flag_Memory >>= 1;
            } else {
                gotmem = true;
                break;
            }
        }
        if (gotmem) {
            fprintf(stderr, "info: alloc %lu bytes\n", Flag_Memory);
        } else {
            errx(EX_OSERR, "could not auto-calloc() memory");
        }
    } else {
        if ((Memory = calloc((size_t) Flag_Memory, sizeof(int))) == NULL)
            err(EX_OSERR, "could not calloc() %lu ints memory", Flag_Memory);
    }

    memset(Memory, 1, Flag_Memory);

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
    fprintf(stderr, "Usage: usemem [-M|-m memory] -t threads\n");
    exit(EX_USAGE);
}

void jkiss_init(void)
{
#ifdef __OpenBSD__
    jkiss_seedX = arc4random() | ((uint64_t) arc4random() << 32);
    jkiss_seedY = arc4random() | ((uint64_t) arc4random() << 32);
#elif defined __linux__ || defined __DARWIN__
    int fd = open("/dev/random", O_RDONLY);
    if (fd == -1)
        err(EX_OSERR, "could not open() /dev/random");

    if (read(fd, &jkiss_seedX, sizeof(jkiss_seedX)) != sizeof(jkiss_seedX))
        err(EX_OSERR, "incomplete read() from /dev/random");

    if (read(fd, &jkiss_seedY, sizeof(jkiss_seedY)) != sizeof(jkiss_seedY))
        err(EX_OSERR, "incomplete read() from /dev/random");

    close(fd);
#else
    errx(1, "jkiss_init() unimplemented on this platform");
#endif
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
