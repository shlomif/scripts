/************************************************************************
 * Penny wise, pound foolish random number generation: entropy hit is
 * only when the srandomdev() call seeds the generator, and only if the
 * count runs up near 16*((2**31)-1) should a reseed be necessary.
 * Otherwise, far more likely that bugs in your custom implementation
 * will reduce or bias the random numbers generated (this one has had
 * several and probably has more), so if such a custom generator is
 * needed, it must have a strong test suite, and the output subjected to
 * statistical tests. Which would be time doubtless better spent doing
 * other things.
 *
 * And, for many hours of coding and debugging, at ~34 extra lines of
 * code, it emits 536870911 random numbers in the range of 12 about 10%
 * faster than just calling random() % randrange (though other ranges
 * will doubtless show varying performance, depending on how well they
 * line up to bit size boundaries). Hooray? A library version that used
 * "all the random bits" and offered dynamic range selection of varying
 * amounts of bits was about *twice* as slow as just calling random()!
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#if defined(linux) || defined(__linux) || defined(__linux__)
#include <sys/types.h>
#include <time.h>
#endif

/* NOTE assumes RAND_MAX 2147483647 */
#define RAND_MAX_BITS 31

/* Bit Twiddling Hacks - Find the log base 2 of an integer with a
 * lookup table */
static const char LogTable256[256] = {
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
    LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};

void emit_usage(void);

int main(int argc, char *argv[])
{
    int ch, plus = 0;
    unsigned int bits, bits_mask, count, i, randrange, randsrc_idx;
    unsigned long randsrc, randval;
    register unsigned int t, tt;        // temporaries for log2 of int

    randrange = 12;
    count = 10000;

    while ((ch = getopt(argc, argv, "h?c:r:p:")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
            emit_usage();
            /* NOTREACHED */
        case 'c':
            if (sscanf(optarg, "%u", &count) != 1)
                errx(EX_DATAERR, "could not parse -c '%s'", optarg);
            break;
        case 'p':
            if (sscanf(optarg, "%d", &plus) != 1)
                errx(EX_DATAERR, "could not parse -p '%s'", optarg);
            break;
        case 'r':
            if (sscanf(optarg, "%u", &randrange) != 1)
                errx(EX_DATAERR, "could not parse -r '%s'", optarg);
            break;
        default:
            emit_usage();
        }
    }

    if (randrange < 2)
        errx(1, "random range must be >1");
    if (randrange > RAND_MAX)
        errx(1, "random range must not exceed %d", RAND_MAX);

    /* Figure out how many bits are necessary to represent the range,
     * and a mask for that range so that appropriately sized chunks can
     * be masked off the random results for each new random value. */
    if (randrange == 2) {
        bits = 1;
        bits_mask = 1;
    } else {
        if ((tt = randrange >> 16)) {
            bits = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
        } else {
            bits = (t =
                    randrange >> 8) ? 8 +
                LogTable256[t] : LogTable256[randrange];
        }
        bits_mask = (1 << ++bits) - 1;
    }

#if defined(linux) || defined(__linux) || defined(__linux__)
    /* something hopefully decent enough */
    srandom(time(NULL) ^ (getpid() + (getpid() << 15)));
#else
    /* assume modern *BSD */
    srandomdev();
#endif

    randsrc = random();
    randsrc_idx = 0;
    for (i = 1; i <= count; i++) {
        randval = (randsrc >> (randsrc_idx++ * bits)) & bits_mask;

        if (RAND_MAX_BITS - randsrc_idx * bits < bits) {
            /* whoops, not enough bits left for next call, refresh */
            randsrc = random();
            randsrc_idx = 0;
        }

        /* If the random range does not match up perfectly with a power
         * of two, some high-bit values will not be usable, reroll. */
        if (randval >= randrange) {
            i--;
            continue;
        }

        printf("%ld\n", (long) randval + plus);
    }

    exit(EXIT_SUCCESS);
}

void emit_usage(void)
{
    errx(EX_USAGE, "-c count -p plusorminus -r randrange");
}
