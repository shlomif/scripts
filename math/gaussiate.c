/*
 * Generate with the GSL some Gaussian random variates. Because reasons.
 */

#include <err.h>
#include <fcntl.h>
#include <float.h>
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

const char *Program_Name;

unsigned long Flag_Count;       // -n
float Flag_Sigma;               // -S

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, fd;
    gsl_rng *gsl_rand;
    uint32_t seed;

    if ((gsl_rand = gsl_rng_alloc(gsl_rng_taus2)) == NULL)
        err(EX_SOFTWARE, "could not gsl_rng_alloc()");

#ifdef __OpenBSD__
    seed = arc4random();

    // since OpenBSD 5.4
    Program_Name = getprogname();
#else
    // assume have a random device and that arc4random sucks on this platform
    fd = open("/dev/random", O_RDONLY);
    if (fd == -1)
        err(EX_OSERR, "could not open /dev/random");
    if (read(fd, &seed, sizeof(seed)) != sizeof(seed))
        err(EX_OSERR, "incomplete read() of /dev/random");

    Program_Name = *argv;
#endif

    while ((ch = getopt(argc, argv, "h?n:S:")) != -1) {
        switch (ch) {

        case 'n':
            Flag_Count = flagtoul(ch, optarg, 1UL, ULONG_MAX);
            break;

        case 'S':
            Flag_Sigma = flagtof(ch, optarg, 0.0, FLT_MAX);
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
    const char *shortname;
#ifdef __OpenBSD__
    shortname = Program_Name;
#else
    if ((shortname = strrchr(Program_Name, '/')) != NULL)
        shortname++;
    else
        shortname = Program_Name;
#endif

    fprintf(stderr, "Usage: %s [-n count] [-S sigma]\n", shortname);

    exit(EX_USAGE);
}
