/* gaussiate - generate with the GSL some Gaussian random variates */

#include <err.h>
#include <fcntl.h>
#include <float.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#define DEFAULT_COUNT 10000UL
#define DEFAULT_SIGMA 1.0

unsigned long Flag_Count;       /* -n */
float Flag_Sigma;               /* -S */

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
#ifndef __OpenBSD__
    int fd;
#endif
    gsl_rng *gsl_rand;
    uint32_t seed;

#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    if ((gsl_rand = gsl_rng_alloc(gsl_rng_taus2)) == NULL)
        err(EX_SOFTWARE, "could not gsl_rng_alloc()");

#ifdef __OpenBSD__
    seed = arc4random();
#else
    /* assume there is a random device otherwise */
    if ((fd = open("/dev/urandom", O_RDONLY)) == -1)
        err(EX_OSERR, "could not open /dev/urandom");
    if (read(fd, &seed, sizeof(seed)) != sizeof(seed))
        err(EX_OSERR, "incomplete read of /dev/urandom");
    close(fd);
#endif

    while ((ch = getopt(argc, argv, "h?n:S:")) != -1) {
        switch (ch) {
        case 'n':
            Flag_Count = flagtoul(ch, optarg, 1UL, ULONG_MAX);
            break;
        case 'S':
            Flag_Sigma = (float) flagtod(ch, optarg, 0.0, (double) FLT_MAX);
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

    if (!isnormal(Flag_Sigma))
        Flag_Sigma = DEFAULT_SIGMA;

    gsl_rng_set(gsl_rand, seed);

    for (unsigned long n = 0; n < Flag_Count; n++)
        printf("%.6f\n", gsl_ran_gaussian(gsl_rand, Flag_Sigma));

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fputs("Usage: gaussiate [-n count] [-S sigma]\n", stderr);
    exit(EX_USAGE);
}
