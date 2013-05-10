/*
 * See sigint-off.c for motiviation.
 */
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

void polonius_polka(int sig);

int main()
{
    /* PORTABILITY - see "signals" chapter in APUE for concerns */
    signal(SIGINT, polonius_polka);

    sleep(6);

    exit(EXIT_SUCCESS);
}

void polonius_polka(int sig)
{
    errx(EX_NOUSER, "oh I am slain");
}
