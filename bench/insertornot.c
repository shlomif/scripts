/*
 * Some job interviews feature sort algorithms; I am not sure why one of
 * mine did, as the position was for IT, where the particulars of sort
 * algorithms never come up, except during job interviews, or
 * discussions about job interviews. Let the language handle it, done. I
 * suppose I could have mentioned that yes, at some point I did copy out
 * a sort algorithm from K&R. I then promptly forgot about it. Why not
 * just use the language sort algo? Subsequent bumbling about through an
 * algorithms book and benchmarking things reveals that while an
 * insertion sort can be written in a few lines of C, qsort(3) beats the
 * pants off of it. Unless, of course, you are in some rare situation
 * and have some special case that needs to be sorted very, very, very
 * quickly (or does not fit in memory) and that sorting is on a critical
 * path that affects the outcome of other very important, time critical
 * things and for some reason the IT staff has been tasked with this
 * task. Contrived, in a word. Even with a mere 10 items to sort,
 * qsort(3) is only half a second slower over a million sorts...eh. Use
 * a language sort function, and don't worry (unless profiling turns up
 * sorting as being a problem, stop asking IT about sorting algos, etc).
 */

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#define NSEC_IN_SEC 1000000000

#define DEFAULT_COUNT 10
#define DEFAULT_TRIALS 100

unsigned long Flag_Count;       // -c
unsigned long Flag_Trials;      // -t

int compare(const void *x, const void *y);
void emit_help(void);
void insertion_sort(uint32_t * list);

int main(int argc, char *argv[])
{
    int ch;
    uint32_t *list;
    struct timespec before, after;
    long double delta_t;

    while ((ch = getopt(argc, argv, "c:h?t:")) != -1) {
        switch (ch) {

        case 'c':
            Flag_Count = flagtoul(ch, optarg, 1UL, LONG_MAX);
            break;

        case 't':
            Flag_Trials = flagtoul(ch, optarg, 1UL, LONG_MAX);
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

    if (Flag_Count == 0)
        Flag_Count = DEFAULT_COUNT;

    if (Flag_Trials == 0)
        Flag_Trials = DEFAULT_TRIALS;

    if (clock_gettime(CLOCK_REALTIME, &before) == -1)
        err(EX_OSERR, "clock_gettime() failed");

    for (unsigned long t = 0; t < Flag_Trials; t++) {
        // insertion sort
        if ((list = calloc(Flag_Count, sizeof(uint32_t))) == NULL)
            err(EX_OSERR, "could not calloc() list of %lu ints", Flag_Count);
        insertion_sort(list);
        free(list);
    }

    if (clock_gettime(CLOCK_REALTIME, &after) == -1)
        err(EX_OSERR, "clock_gettime() failed");
    delta_t =
        (after.tv_sec - before.tv_sec) + (after.tv_nsec -
                                          before.tv_nsec) /
        (long double) NSEC_IN_SEC;
    fprintf(stderr, "ins\t%.6Lf\n", delta_t);

    if (clock_gettime(CLOCK_REALTIME, &before) == -1)
        err(EX_OSERR, "clock_gettime() failed");

    for (unsigned long t = 0; t < Flag_Trials; t++) {
        // qsort
        if ((list = malloc(Flag_Count * sizeof(uint32_t))) == NULL)
            err(EX_OSERR, "could not malloc() list of %lu ints", Flag_Count);
        for (unsigned long c = 0; c < Flag_Count; c++) {
            list[c] = arc4random();
        }
        qsort(list, Flag_Count, sizeof(uint32_t), compare);
        free(list);
    }

    if (clock_gettime(CLOCK_REALTIME, &after) == -1)
        err(EX_OSERR, "clock_gettime() failed");
    delta_t =
        (after.tv_sec - before.tv_sec) + (after.tv_nsec -
                                          before.tv_nsec) /
        (long double) NSEC_IN_SEC;
    fprintf(stderr, "qsort\t%.6Lf\n", delta_t);

    // DBG to check that the list is actually properly sorted
/*  for (unsigned long c = 0; c < Flag_Count; c++) {
        printf("%u\n", list[c]);
    } */

    exit(EXIT_SUCCESS);
}

int compare(const void *x, const void *y)
{
    if (*(uint32_t *) x < *(uint32_t *) y) {
        return -1;
    } else if (*(uint32_t *) x > *(uint32_t *) y) {
        return 1;
    } else {
        return 0;
    }
}

void emit_help(void)
{
    fprintf(stderr, "Usage: insertornot\n");
    exit(EX_USAGE);
}

void insertion_sort(uint32_t * list)
{
    for (unsigned long c = 0; c < Flag_Count; c++) {
        unsigned long x;
        uint32_t newval = arc4random();

        for (x = c; x > 0 && list[x - 1] > newval; x--) {
            list[x] = list[x - 1];
        }
        list[x] = newval;
    }
}
