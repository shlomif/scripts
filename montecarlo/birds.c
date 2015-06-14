/*
 * A "Digital Dice" by Paul Nahin problem, originally birds looking at one
 * another but carified by Nahin so I'm going back to the birds.
 *
 *       ... I only got 2/3rds of the way through the problem, though.
 */

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#include "aaarghs.h"

#define DEFAULT_BIRDS 3UL
#define MINBIRDS 2UL

unsigned long Flag_Birds;       // -n
unsigned long Flag_Trials;      // -c

unsigned long neighbor(const unsigned long idx, const double *locs,
                       const unsigned long locsmax);

int main(int argc, char *argv[])
{
    double *locs;
    int ch;
    unsigned long mncount = 0;

    while ((ch = getopt(argc, argv, "c:h?n:")) != -1) {
        switch (ch) {

        case 'c':
            Flag_Trials = flagtoul(ch, optarg, 1UL, LONG_MAX);
            break;

        case 'n':
            Flag_Birds =
                flagtoul(ch, optarg, MINBIRDS, (unsigned long) CHAR_MAX);
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

    if (Flag_Birds == 0)
        Flag_Birds = DEFAULT_BIRDS;

    if (Flag_Trials == 0)
        Flag_Trials = TRIALS;

    if ((locs = calloc(Flag_Birds, sizeof(double))) == NULL)
        err(EX_OSERR, "could not calloc() list of locations");

    for (unsigned long i = 0; i < Flag_Trials; i++) {
        unsigned long birdidx;

        for (unsigned long b = 0; b < Flag_Birds; b++) {
            unsigned long c;
            double newval = arc4random() / (double) UINT32_MAX;

            // insertion sort into the thus sorted list of locations
            for (c = b; c > 0 && locs[c - 1] > newval; c--) {
                locs[c] = locs[c - 1];
            }
            locs[c] = newval;
        }

        birdidx = (unsigned long) arc4random_uniform((uint32_t) Flag_Birds);

        if (birdidx == 0) {
            if (neighbor(birdidx + 1, locs, Flag_Birds) == 0)
                mncount++;
        } else if (birdidx == Flag_Birds - 1) {
            if (neighbor(birdidx - 1, locs, Flag_Birds) == Flag_Birds - 1)
                mncount++;

        } else if ((neighbor(birdidx, locs, Flag_Birds) == birdidx - 1
                    && neighbor(birdidx - 1, locs, Flag_Birds) == birdidx)
                   || (neighbor(birdidx, locs, Flag_Birds) == birdidx + 1
                       && neighbor(birdidx + 1, locs, Flag_Birds) == birdidx)) {
            mncount++;
        }
    }

    printf("%.4f\n", mncount / (double) Flag_Trials);

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: birds [-c trials] [-n numbirds]\n");
    exit(EX_USAGE);
}

unsigned long neighbor(const unsigned long idx, const double *locs,
                       const unsigned long locsmax)
{
    if (idx == 0) {
        return 1;
    } else if (idx == locsmax - 1) {
        return idx - 1;
    } else {
        return (locs[idx] - locs[idx - 1] <
                locs[idx + 1] - locs[idx]) ? idx - 1 : idx + 1;
    }
}
