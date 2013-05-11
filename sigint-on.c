/*
 * See sigint-off.c for motiviation.
 */

#include <sys/types.h>

#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <sysexits.h>

void polonius_polka(int sig);

int main()
{
    /* PORTABILITY - see "signals" chapter in APUE for concerns; copy
     * sigaction code from obdurate.c if OS needs that istead of
     * signal().
     */
    signal(SIGINT, polonius_polka);

    for (;;);

    exit(EXIT_SUCCESS);
}

void polonius_polka(int sig)
{
    errx(EX_NOUSER, "oh I am slain");   // 128+sig does not change what zsh sees
}
