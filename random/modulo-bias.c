/* Test for modulo bias in poor random number
 * generation; the bias does not appear where RAND_MAX
 * is much larger than m. For a three-sided dice roll,
 * the last RAND_MAX to fail what I think is a chi-
 * squared 0.05 test is 257; given a system RAND_MAX
 * of a much larger value, this would likely only be
 * of concern if the modulo m is very, very large, or
 * if an 8-bit system is being used. Better results
 * are obtained where m evenly divides RAND_MAX;
 * RAND_MAX of 100000 and m = 4 only fails at 62 and
 * below. (Assuming no errors on my part).
 *
 * When testing, an ample number of tests is necessary
 * to make a safe determination; this is balanced by
 * the need for the test results to not take too long
 * to produce.
 *
 * CFLAGS=`pkg-config --libs --cflags gsl` make ...
 */

#include <gsl/gsl_cdf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define RANGE 3
#define TESTNUM 99999
#define EXPECTED TESTNUM / RANGE
#define DEG_OF_FREE RANGE - 1

#define FITNESS 0.05

#define randof(x) ((rand() % x) % RANGE)

unsigned int results[RANGE];

int main(void)
{
    int yesno;
    unsigned int i, randmax, tnum;
    double sq, q;

    /* pseudorandom within pseudorandom, per Dune */
    srand(arc4random());

    for (randmax = RANGE; randmax < 1000;
         randmax += 1) {
        for (tnum = 0; tnum < TESTNUM; tnum++) {
            results[randof(randmax)]++;
        }
        sq = 0.0;
        for (i = 0; i < RANGE; i++) {
            sq +=
                pow((double) results[i] - EXPECTED,
                    2) / EXPECTED;
            results[i] = 0;
        }
        q = gsl_cdf_chisq_Q(sq, DEG_OF_FREE);
        yesno = q >= FITNESS ? 1 : 0;
        printf("%d %d\n", randmax, yesno);
    }

    exit(0);
}
