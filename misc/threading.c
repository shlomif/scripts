/*
 * Mostly so can have a baseline of what a threaded process looks like in ps(1)
 * and whatnot, etc.
 */

#include <err.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

void *busy(void *arg);
void *waiting(void *arg);

int main(void)
{
/* Vega and Altair, rather forgotten by most moderns with their lights.
 * Otherwise not very portable, given the distances involved, or the varying
 * struct forms between different OS. */
    pthread_t weaver_maid, ox_driver;
    int pattern = 1431655765;
    int *driver;

    if (pthread_create(&weaver_maid, NULL, waiting, &pattern) != 0)
        err(EX_OSERR, "could not make thread");
    if (pthread_create(&ox_driver, NULL, busy, "driver") != 0)
        err(EX_OSERR, "could not make thread");
    pthread_join(weaver_maid, (void *) &driver);

    printf("pattern %d is %d\n", pattern, *driver);

    exit(EXIT_SUCCESS);
}

void *busy(void *arg)
{
    fprintf(stderr, "thread tid %p is %s\n", pthread_self(), (char *) arg);
    for (;;);
    return ((void *) 0);
}

void *waiting(void *arg)
{
    struct timespec yawn = { 21, 42 };
    fprintf(stderr, "thread tid %p waits\n", pthread_self());
    nanosleep(&yawn, NULL);
    pthread_exit(arg);
}
