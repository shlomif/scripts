/* Simple C delta time calculator */

#include <sys/time.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#define NSEC_IN_SEC 1000000000

int main(void)
{
    struct timespec before, after;
    long double delta_t;

    if (clock_gettime(CLOCK_REALTIME, &before) == -1)
        err(EX_OSERR, "clock_gettime() failed");

    sleep(3);                   // real code here

    if (clock_gettime(CLOCK_REALTIME, &after) == -1)
        err(EX_OSERR, "clock_gettime() failed");

    /* One handy sanity check might be to confirm that time has actually
     * advanced, a concern tied up with what time sync protocol is being used,
     * whether that protocol is operating correct, and etc. See also
     * CLOCK_MONOTONIC in clock_gettime(2), etc. */
    delta_t =
        (after.tv_sec - before.tv_sec) + (after.tv_nsec -
                                          before.tv_nsec) /
        (long double) NSEC_IN_SEC;
    fprintf(stderr, "delta %.6Lf\n", delta_t);

    /* for compat with -fstack-protector-all, cannot just return */
    exit(EXIT_SUCCESS);
}
