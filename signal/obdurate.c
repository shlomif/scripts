/*
 * Ignore most common signals. Handy to test things that will require up
 * to a SIGKILL to clear from the process table (like, hypothetically, a
 * horribly wedged LDAP server). Complicated things like forking child
 * processes and then waiting on those and also interacting with
 * associated signals not supported.
 *
 * PID emitted to stdout, all other output should be to stderr.
 *
 * If stdin is not a TTY, a temporary file will be blocked on. This temp
 * file will not be cleaned up for obvious reasons.
 *
 * XXX figure out way just to test whether stdin "is open".
 */

#include <sys/types.h>

#include <err.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

void whine(int sig);

int main(void)
{
    char buf[BUFSIZ];
    char tmp_filename[] = "/tmp/obd.XXXXXXXXXX";
    int nfds;
    struct pollfd pfd[1];
    struct sigaction act;

    /*
     * Setup signal handling
     */
//    act.sa_handler = SIG_IGN;   // just ignore the signal
    act.sa_handler = whine;     // whine about the signal

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
    // might happen if stderr closed for some reason?
    if (sigaction(SIGPIPE, &act, NULL) != 0)
        err(EX_OSERR, "sigaction failed");

    act.sa_flags |= SA_RESTART;
    if (sigaction(SIGALRM, &act, NULL) != 0)
        err(EX_OSERR, "sigaction failed");

    /*
     * (Blocking) I/O is Tricky
     */

    // Avoid busy loop if cannot block on input (e.g. started in background)
    if (isatty(STDIN_FILENO)) {
        pfd[0].fd = STDIN_FILENO;
    } else {
        if ((pfd[0].fd = mkstemp(tmp_filename)) == -1)
            err(EX_IOERR, "mkstemp failed to create tmp file");
    }
    pfd[0].events = POLLPRI;

    printf("info: pid %d\n", getpid());

    for (;;) {
        nfds = poll(pfd, 1, 60 * 1000);
        if (nfds == -1 || (pfd[0].revents & (POLLERR | POLLHUP | POLLNVAL)))
            warnx("poll error");
        else if (nfds == 0)
            //   warnx("timeout");
            ;
        else if (read(pfd[0].fd, buf, sizeof(buf)) == -1)
            warnx("could not read");
    }

    exit(EXIT_SUCCESS);
}

void whine(int sig)
{
    warnx("caught signal %d at %ld", sig, (long) time(NULL));
}
