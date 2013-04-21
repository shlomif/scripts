/* Simple C delta time calculator */

#include <sys/time.h>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#define USEC_IN_SEC 1000000

int main()
{
    struct timeval before, after;
    double delta_t;

    assert(gettimeofday(&before, NULL) != -1);

    sleep(3);                   // real code here

    assert(gettimeofday(&after, NULL) != -1);

    delta_t = after.tv_sec - before.tv_sec;
    delta_t += (float) (after.tv_usec - before.tv_usec) / USEC_IN_SEC;
    fprintf(stderr, "delta %.3f\n", delta_t);

    return (0);
}
