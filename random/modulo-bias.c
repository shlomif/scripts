/* Test for modulo bias in poor random number generation; the bias does not
 * appear where RAND_MAX is much larger than m. For a three-sided dice roll,
 * the last RAND_MAX to fail what I think is a chi-squared 0.05 test is
 * 257; given a system RAND_MAX of a much larger value, this would likely
 * only be of concern if the modulo m is very, very large, or if an 8-bit
 * system is being used. Better results are obtained where m evenly divides
 * RAND_MAX; RAND_MAX of 100000 and m = 4 only fails at 62 and below.
 * (Assuming no errors on my part).
 *
 * Actually, the exact numbers depend on the rand() implementation and the
 * seed; Mac OS X rand() produced the 257 for an arbitrary seed; this
 * version tries out multiple seeds (TRIALS) and emits what is likely the
 * maximum failure of the fitness test for that seed. However! It still
 * remains that if RAND_MAX is much larger than the range, there is no
 * (statistically significant) modulo bias.
 *
 * CFLAGS=`pkg-config --libs --cflags gsl` make ...
 */

#include <gsl/gsl_cdf.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* How many results to generate (3 == (0,1,2)) and how many tests to run;
 * TESTNUM will need to be high enough to distinguish signal from the noise,
 * but not too long that too much CPU is wasted. */
#define RANGE 3
#define TESTNUM 99999

/* CHI^2 test foo, assuming uniform distribution of the RANGE values over
 * the TESTNUM runs, and 95% confidence. */
#define EXPECTED TESTNUM / (double) RANGE
#define DEG_OF_FREE RANGE - 1
#define FITNESS 0.05

/* This may have to go up if larger RANGE are being tested, e.g. some huge
 * prime vs. some other huge prime, or otherwise scale in some relation to
 * RANGE and randmax. */
#define NOBIAS_RUN 100

/* How many (hopefully) different random seeds to do the test under. */
#define TRIALS 500

#define randof(x) ((rand() % x) % RANGE)

unsigned int results[RANGE];

int main(void)
{
    unsigned int i, maxfail, nobias, randmax, testnum, trialnum;
    double sq;

    //setvbuf(stdout, (char *)NULL, _IOLBF, (size_t) 0);

    for (trialnum = 0; trialnum < TRIALS; trialnum++) {
        /* pseudorandom within pseudorandom, per Dune */
        srand(arc4random());

        nobias = 0;

        for (randmax = RANGE; randmax < UINT_MAX; randmax += 1) {
            for (testnum = 0; testnum < TESTNUM; testnum++) {
                results[randof(randmax)]++;
            }
            sq = 0.0;
            for (i = 0; i < RANGE; i++) {
                sq += pow((double) results[i] - EXPECTED, 2) / EXPECTED;
                results[i] = 0; // reset for the next test
            }
            if (gsl_cdf_chisq_Q(sq, DEG_OF_FREE) <= FITNESS) {
                maxfail = randmax;
                nobias = 0;
            } else {
                if (nobias++ >= NOBIAS_RUN) {
                    break;
                }
            }
        }
        printf("%d\n", maxfail);
    }

    exit(0);
}
