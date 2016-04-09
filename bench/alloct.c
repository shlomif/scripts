/*
 * Test whether malloc zeros memory or not (it does, on certain modern OS).
 * (This is not really a benchmark script, though one may need to 'usemem'
 * to fill the memory on a system with random values.)
 *
 * Compile w/o optimization, probably.
 */

#include <err.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#define MOSTMEMPOSSIBLE ( (ULONG_MAX < SIZE_MAX) ? ULONG_MAX : SIZE_MAX )

unsigned long memsize;

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    int *foop;
    // this is what is at risk of not being zero'd; things passed through
    // malloc are on modern OS (Mac OS X 10.11.4, OpenBSD 5.9, etc)
    int bar[999999];
    unsigned long i;

    for (i = 0; i < 999999; i++) {
        if (bar[i] != 0)
            abort();
    }

    while ((ch = getopt(argc, argv, "h?m:")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
        case 'm':
            memsize = flagtoul(ch, optarg, 1UL, MOSTMEMPOSSIBLE);
            break;
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    /*
    if (memsize == 0)
        memsize = MOSTMEMPOSSIBLE / sizeof(int);

    while (memsize) {
        if ((foop = malloc(memsize * sizeof(int))) == NULL) {
            memsize >>= 1;
        } else {
            break;
        }
    }

    if (foop) {
        fprintf(stderr, "info: auto-alloc picks %lu ints\n", memsize);
    } else {
        err(EX_OSERR, "could not auto-malloc() %lu ints", memsize);
    }
    foop[1234] = 9;

    for (i = 0; i < memsize; i++) {
        if (*foop++ != 0)
            abort();
    }
    */

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: alloct [-m max-memory-size]\n");
    exit(EX_USAGE);
}
