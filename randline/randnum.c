/* Generates random values (in the range of 0 through one minus the -r
 * flag, to the order of -c values. Is (variously)efficient with the
 * pseudorandom data, depending on the number of bits required, how
 * well that fits into the 31 bits returned by random(3), etc. (Using
 * *all* the random bits would require storing the results of two
 * random(3) calls, and dealing with the complication of masks that need
 * to pull from the end of one and the beginning of the next.)
 *
 * On the plus side, it is about three times faster than 'print
 * int(rand(12)) for 1..10000000' Perl code, though did take somewhat
 * longer code up.
 *
 * May need -lm if compiling with gcc to avoid `undefined reference' to
 * things from <math.h>, or other alterations if compiling somewhere
 * besides a Mac OS X or OpenBSD system.
 */

#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#define RAND_BITS_MAX 31

void emit_usage(void);

int main(int argc, char *argv[])
{
    int ch, ret;
    unsigned int bits, bits_mask, count, i, randrange, randsrc_idx;
    unsigned long randsrc, randval;

    randrange = 12;
    count = 10000;

    while ((ch = getopt(argc, argv, "h?c:r:")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
            emit_usage();
            /* NOTREACHED */
        case 'c':
            ret = sscanf(optarg, "%u", &count);
            if (ret != 1)
                errx(EX_DATAERR, "could not parse -c '%s'", optarg);
            break;
        case 'r':
            ret = sscanf(optarg, "%u", &randrange);
            if (ret != 1)
                errx(EX_DATAERR, "could not parse -r '%s'", optarg);
            break;
        }
    }

    /* A range of N will need log2 of that (rounded up) bits to store
     * all the possible values. */
    bits = (int) ceil(log2((double) randrange));
    if (bits > RAND_BITS_MAX)
        errx(1, "random range is too large for random(3)");
    bits_mask = (1 << bits) - 1;

    srandomdev();

    randsrc = random();
    randsrc_idx = 0;
    for (i = 1; i <= count; i++) {
        if (randsrc_idx * (bits + 1) >= RAND_BITS_MAX) {
            randsrc = random();
            randsrc_idx = 0;
        }
        randval =
            (unsigned int) (randsrc >> (randsrc_idx++ * bits)) & bits_mask;
        /* If the random range does not match up perfectly with a power
         * of two, some high-bit values will not be usable. */
        if (randval >= randrange) {
            i--;
            continue;
        }
        printf("%ld\n", randval);
    }

    return 0;
}

void emit_usage(void)
{
    errx(EX_USAGE, "-c count -r rand");
}
