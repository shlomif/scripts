/*
 # (Ab)uses memory via the allocation of a block of memory, with a given
 # number of worker threads that then read and write values randomly
 # within that space. This code was motivated by the fact that while
 # `perl -e '1 while 1' &` can run up the CPU, one is more likely to run
 # into fork limits than CPU bottlenecks on modern multi-processor
 # systems. This code better exercises a system. Basically, it offers a
 # means to bog a system down, presumably for testing purposes, race
 # condition exploration, or to annoy the local sysadmin.
 #
 # System resource limits may need to be adjusted; see login.conf(5) on
 # OpenBSD, for example. There may also be per-process limitations. A
 # good way to bog the system down is to run something like:
 #
 #   for i in ...; do ./usemem -M -t ... & done
 #
 # Where the number of processes to launch and the thread count are
 # suitable to the number of CPUs or desired level of insanity. A timed
 # pkill(1) or easy access to reset the test system should be arranged
 # for, given that too many instances of this program may render a
 # system quite inoperative--a desktop linux system `vmstat 1` recorded
 # 12 million context switches in an entry, presumably over some time
 # period longer than a second, given how poorly that system was
 # performing. Shortly thereafter, the display froze up, and the system
 # had to be power cycled (I could still ping the box).
 #
 # The original intent of this code was to try to trigger a race
 # condition on:
 #
 #   alarm(...);
 #   somethingthatblocksforever();
 #   alarm(0);
 #     // -- paraphrased from Stevens, APUE (1st edition), p. 286.
 #
 # whereby the system is made busy enough that the SIGALRM happens
 # before the somethingthatblocks() call can be started, but after the
 # alarm(...) is established, so that the process then blocks forever.
 # It is perhaps far more likely that the system will be rendered
 # unusable (and monitoring notice this) than to hit this unlikely edge
 # case. If a system has been inordinately busy or slow, a reboot may
 # well be in order as soon as is practical. This will eliminate the
 # possiblilty that any process remains stuck on some rare edge case
 # such as the above.
 #
 # Improvements may be to malloc() multiple blocks instead of just one,
 # at the cost of complicating this code (instead, simply launch
 # multiple instances of this script). Another option would be for
 # thread sleep, so the workers could stall from time to time, and the
 # OS thus perhaps swaps out the idle process. With multiple processes
 # running actively, the Linux out-of-memory killer just starts blasting
 # away. Idled processes brought back into memory could induce swapping,
 # which would be especially slow on systems with slow disk. (This could
 # be implemented via external SIG{STOP,CONT} signals against a list of
 # instances of this code, I'd wager.)
 #
 # Portability: -M does sensible things on Linux and OpenBSD. On Mac OS
 # X, it allocates a large number, well more than the physical memory
 # available, and then the kernel thrashes around trying to allocate all
 # that. So, on Mac OS X, use -m and specify the amount of memory to
 # allocate.
 */

#ifdef __linux__
#define _BSD_SOURCE
#define _GNU_SOURCE
#include <getopt.h>
#endif

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
#include <unistd.h>

#include "../montecarlo/aaarghs.h"

#define MAX_THREADS 640UL

#define MOSTMEMPOSSIBLE ( (ULONG_MAX < SIZE_MAX) ? ULONG_MAX : SIZE_MAX )

bool Flag_Auto_Mem;             // -M
unsigned long Flag_Memory;      // -m (amount, though internally # of ints)
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
pthread_cond_t Done_Malloc = PTHREAD_COND_INITIALIZER;
pthread_cond_t Never_Happens = PTHREAD_COND_INITIALIZER;

void emit_help(void);
void jkiss_init(void);
uint64_t jkiss_rand64(void);
unsigned long jkiss_randof(const unsigned long max);
void *worker(void *unused);

int main(int argc, char *argv[])
{
    int ch;
    pthread_t *tids;

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

    /* Ahh gee em. Complicated. See, if you malloc() the most memory
     * possible, then there is no space for the threads. So must spin
     * the threads up first, and then binary search realloc() to find
     * the most memory possible. Presumably other instances of this
     * program or other processes will be run to eat up any remaining
     * memory on the system.
     */

    jkiss_init();

    if ((tids = calloc(sizeof(pthread_t), Flag_Threads)) == NULL)
        err(EX_OSERR, "could not calloc() threads list");

    for (unsigned long i = 0; i < Flag_Threads; i++) {
        if (pthread_create(&tids[i], NULL, worker, NULL) != 0)
            err(EX_OSERR, "could not pthread_create() thread %lu", i);
    }

    if (Flag_Auto_Mem) {
        size_t lo = 1;
        size_t hi = MOSTMEMPOSSIBLE / sizeof(int);
        int *mem = NULL;

        while (lo <= hi) {
            size_t mid = lo + (hi - lo) / 2;
            if ((mem = realloc(mem, mid * sizeof(int))) == NULL) {
                hi = mid - 1;
            } else {
                lo = mid + 1;
                Flag_Memory = mid;
                Memory = mem;
            }
        }
        fprintf(stderr, "info: auto-alloc picks %lu ints\n", Flag_Memory);
    } else {
        Flag_Memory /= sizeof(int);
        if ((Memory = malloc(Flag_Memory * sizeof(int))) == NULL)
            err(EX_OSERR, "could not malloc() %lu ints\n", Flag_Memory);
    }

    memset(Memory, 1, Flag_Memory * sizeof(int));

    pthread_cond_broadcast(&Done_Malloc);

    pthread_mutex_lock(&Lock);
    pthread_cond_wait(&Never_Happens, &Lock);
    pthread_mutex_unlock(&Lock);

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
#elif defined(__DARWIN__) || defined(__linux__)
    int fd = open("/dev/random", O_RDONLY);
    if (fd == -1)
        err(EX_OSERR, "could not open() /dev/random");

    if (read(fd, &jkiss_seedX, sizeof(jkiss_seedX)) != sizeof(jkiss_seedX))
        err(EX_OSERR, "incomplete read() from /dev/random");

    if (read(fd, &jkiss_seedY, sizeof(jkiss_seedY)) != sizeof(jkiss_seedY))
        err(EX_OSERR, "incomplete read() from /dev/random");

    close(fd);
#else
    // TODO clang on Mac OS X doesn't see the __DARWIN__ thing??
    warnx("jkiss_init() unimplemented on this platform");
    jkiss_seedX = 123456789123ULL;
    jkiss_seedY = 987654321987ULL;
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
    /* Do not much care about modulo bias, as most malloc()d spaces are
     * doubtless well below SIZE_MAX, and if there were any bias it
     * probably isn't anything the CPU cache code could optimize for.
     */
    return (unsigned long) jkiss_rand64() % max;
}

void *worker(void *unused)
{
    unsigned long idx;

    pthread_mutex_lock(&Lock);
    pthread_cond_wait(&Done_Malloc, &Lock);
    pthread_mutex_unlock(&Lock);

    for (;;) {
        idx = jkiss_randof(Flag_Memory);
        Memory[idx] = (idx % 2 == 1)
            ? Memory[jkiss_randof(Flag_Memory)]
            : rand();
    }

    /* NOTREACHED */
}
