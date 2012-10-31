/*
 * Ignore most common signals. Handy to test things that will require up
 * to a SIGKILL to clear from the process table. (Complicated things
 * like forking child processes and then waiting on those and also
 * interacting with associated signals not supported.)
 */

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <sys/types.h>
#include <unistd.h>

void whine(int sig);

int main()
{
    int c;
    struct sigaction act;

//    act.sa_handler = SIG_IGN;   // ignore the signal
    act.sa_handler = whine;     // whine about the signal

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(SIGTERM, &act, NULL) != 0)
        err(EX_OSERR, NULL);
    if (sigaction(SIGINT, &act, NULL) != 0)
        err(EX_OSERR, NULL);
    if (sigaction(SIGHUP, &act, NULL) != 0)
        err(EX_OSERR, NULL);
    if (sigaction(SIGUSR1, &act, NULL) != 0)
        err(EX_OSERR, NULL);
    if (sigaction(SIGUSR2, &act, NULL) != 0)
        err(EX_OSERR, NULL);
    if (sigaction(SIGPIPE, &act, NULL) != 0)
        err(EX_OSERR, NULL);

    act.sa_flags |= SA_RESTART;
    if (sigaction(SIGALRM, &act, NULL) != 0)
        err(EX_OSERR, NULL);

    printf("info: pid %ld\n", (long int) getpid());

    for (;;) {
        while ((c = getchar()) != EOF) {
            putchar(c);
        }
    }

    exit(EXIT_SUCCESS);
}

void whine(int sig)
{
    warnx("caught signal %d", sig);
}
