/* simple C delta time calculator - possibly handy for benchmark needs,
 * assuming what is being benchmarked runs for a sufficient period of
 * time though consider instead using gprof or similar:
 *
 *   gcc -pg ...
 *   ./...
 *   gprof ... gmon.out
 *
 * or investigating performance via dtrace or sysdig type OS tools. yet
 * another option is the perl Dumbbench module, which uses statistical
 * foo on the wallclock time between various runs */

#include <sys/resource.h>
#include <sys/time.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#define NSEC_IN_SEC 1000000000
#define MSEC_IN_SEC    1000000

void testcode(void);
void deltaof(void (*thecall) (), const char *name);

int main(void)
{
    deltaof(testcode, "sleep 3");

    exit(EXIT_SUCCESS);
}

void deltaof(void (*thecall) (), const char *name)
{
    long double delta_t, delta_user, delta_sys;
    struct rusage use_before, use_after;
    struct timespec before, after;

    if (getrusage(RUSAGE_SELF, &use_before) == -1)
        err(EX_OSERR, "getrusage() failed");
    if (clock_gettime(CLOCK_REALTIME, &before) == -1)
        err(EX_OSERR, "clock_gettime() failed");

    thecall();

    if (clock_gettime(CLOCK_REALTIME, &after) == -1)
        err(EX_OSERR, "clock_gettime() failed");
    if (getrusage(RUSAGE_SELF, &use_after) == -1)
        err(EX_OSERR, "getrusage() failed");

    /* One handy sanity check might be to confirm that time has actually
     * advanced, a concern tied up with what time sync protocol is being used,
     * whether that protocol is operating correct, and etc. See also
     * CLOCK_MONOTONIC in clock_gettime(2), etc. */
    delta_t =
        (after.tv_sec - before.tv_sec) + (after.tv_nsec -
                                          before.tv_nsec) /
        (long double) NSEC_IN_SEC;

    delta_user =
        (use_after.ru_utime.tv_sec - use_before.ru_utime.tv_sec) +
        (use_after.ru_utime.tv_usec -
         use_before.ru_utime.tv_usec) / (long double) MSEC_IN_SEC;
    delta_sys =
        (use_after.ru_stime.tv_sec - use_before.ru_stime.tv_sec) +
        (use_after.ru_stime.tv_usec -
         use_before.ru_stime.tv_usec) / (long double) MSEC_IN_SEC;

    fprintf(stderr, "%s\t%.6Lf real %.6Lf user %.6Lf system\n", name, delta_t,
            delta_user, delta_sys);
}

/* and presumably this would contain some non-trival amount of work to
 * be measured. */
void testcode(void)
{
    sleep(3);
}
