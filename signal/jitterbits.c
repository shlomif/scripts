/*
 * Entropy from timer delay test.
 *
 *   CFLAGS="-std=c99" make jitterbits
 *   for i in {1..4}; do ./jitterbits > $TMPDIR/jbout.$i &| done
 *   
 *   ... (wait a while to collect data, then pkill the jitterbits)
 *
 *   cat $TMPDIR/jbout.* | r-fu equichisq
 *
 * Though this method may not be portable, will need testing, etc.
 */

#include <sys/time.h>

#include <err.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

/* global mostly for the free zeroing of all the various struct fields */
struct itimerval iTimer;

uint8_t Rand, Whence;
#define ROLLOVER 8

void handle_alarm(int unused);

int main(void)
{
    char tmp_filename[] = "/tmp/jbsmall.XXXXXXXXXX";
    struct pollfd pfd[1];

    setvbuf(stdout, (char *)NULL, _IOLBF, (size_t) 0);

    if (signal(SIGALRM, handle_alarm) == SIG_ERR)
        err(EX_OSERR, "could not setup signal() handler");

    /* repeating N-second timer... */
    iTimer.it_value.tv_sec = 1;
    iTimer.it_interval.tv_sec = 1;
    if (setitimer(ITIMER_REAL, &iTimer, NULL) == -1)
        err(EX_OSERR, "could not setitimer()");

    /* A real application would either be waiting for user input, or as an
     * entropy gathering daemon, probably polling and also reading random
     * timings off of network interrupts, etc, in which case having a
     * thread for that might be handy, as then one of the timer slots is
     * not tied up, etc. */

    // Avoid busy loop if cannot block on input (e.g. started in background)
    if (isatty(STDIN_FILENO)) {
        pfd[0].fd = STDIN_FILENO;
    } else {
        fprintf(stderr, "notice: doing mkstemp to create file to poll...\n");
        if ((pfd[0].fd = mkstemp(tmp_filename)) == -1)
            err(EX_IOERR, "mkstemp failed to create tmp file");
    }
    pfd[0].events = POLLPRI;
    for (;;) {
        poll(pfd, 1, 60 * 10000);
    }

    /* NOTREACHED */
    exit(1);
}

void handle_alarm(int unused)
{
    struct timespec now;
    if (clock_gettime(CLOCK_REALTIME, &now) == -1)
        err(EX_OSERR, "clock_gettime() failed");
    Rand ^= (now.tv_nsec & 1) << Whence++;
    if (Whence >= ROLLOVER) {
        Whence = 0;
        printf("%u\n", Rand);
    }
}
