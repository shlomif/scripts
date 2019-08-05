/* randrand - how random is rand(), mostly for comparison with the
 * random numbers signal/jitterbits.c produces. produces 256 different
 * random values which in an ideal world should be evenly distributed
 *
 *   ./randrand | r-fu equichisq
 *
 * even with gsl_rng_uniform_int() and the "maximally equidistributed"
 * Tausworthe generator (mark II), the p-values over 100 runs of this
 * version of the program on Mac OS X vary wildly, with a mean of 0.55
 * and standard deviation of 0.28 (and similar stats from OpenBSD) */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include <gsl/gsl_rng.h>

#define TIMES 10000000

int Return_Value = EXIT_SUCCESS;

int main(void)
{
    unsigned long i;
    gsl_rng *gslrng;

#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    if ((gslrng = gsl_rng_alloc(gsl_rng_taus2)) == NULL)
        err(EX_SOFTWARE, "call to gsl_rng_alloc() failed");
    gsl_rng_set(gslrng, arc4random());

    /* for instead rand() */
    //srandomdev();
    for (i = 0; i < TIMES; i++) {
        /* for instead rand() */
        //printf("%u\n", rand() % 256);

        /* for instead arc4random() */
        //printf("%u\n", arc4random() % 256);

        printf("%lu\n", gsl_rng_uniform_int(gslrng, 256));

        /* reseeding influences things? (no?) */
        //if (i % 100 == 0)
        //  gsl_rng_set(gslrng, arc4random());
    }

    exit(Return_Value);
}
