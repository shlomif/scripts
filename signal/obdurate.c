/* obdurate - ignore most common signals. handy to test things that will
 * require up to a SIGKILL to clear from the process table (like,
 * hypothetically, a horribly wedged LDAP server) */

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <sysexits.h>
#include <unistd.h>

char buf[1024];

void whine(int sig);

int main(void)
{
    int fds[2];
    struct sigaction act;

    if (pipe(fds) != 0)
        err(1, "pipe failed ??");

    /* TWEAK pick which behavior you want */
    //act.sa_handler = SIG_IGN;
    act.sa_handler = whine;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(SIGTERM, &act, NULL) != 0)
        err(EX_OSERR, "sigaction failed");
    if (sigaction(SIGINT, &act, NULL) != 0)
        err(EX_OSERR, "sigaction failed");
    if (sigaction(SIGHUP, &act, NULL) != 0)
        err(EX_OSERR, "sigaction failed");
    if (sigaction(SIGUSR1, &act, NULL) != 0)
        err(EX_OSERR, "sigaction failed");
    if (sigaction(SIGUSR2, &act, NULL) != 0)
        err(EX_OSERR, "sigaction failed");
    if (sigaction(SIGPIPE, &act, NULL) != 0)
        err(EX_OSERR, "sigaction failed");

    act.sa_flags |= SA_RESTART;
    if (sigaction(SIGALRM, &act, NULL) != 0)
        err(EX_OSERR, "sigaction failed");

    setvbuf(stdout, (char *) NULL, _IOLBF, (size_t) 0);
    warnx("pid %d", getpid());

    while (1)
        read(fds[0], buf, 1024);

    return 1;
}

void whine(int sig)
{
    printf("caught signal %d\n", sig);
}
