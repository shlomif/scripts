/*
 * So this is due to the following puzzler where `tail -f` behaves
 * differently than code that catches SIGINT, wherein the ; echo ...
 * is not run by the shell as expected after the `tail -f`.
 *
 *   $ ./sigint ; echo asdf
 *   pid=2032 ppid=18042 tcgetpgrp=2032
 *   ^Csigint: oh I am slain
 *   asdf
 *   $ tail -n1 -f /etc/passwd; echo ya ya
 *   _postgresql:*:503:503:PostgreSQL Manager:/var/postgresql:/bin/sh
 *   ^C
 *   $ 
 *
 * However, the behavior (whether the echo is run or not) appears to vary
 * by the OS or shell? Getting confounding results...
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
    /* no signal handler */

    sleep(6);

    exit(EXIT_SUCCESS);
}

void polonius_polka(int sig)
{
    errx(EX_NOUSER, "oh I am slain");
}
