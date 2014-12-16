/*
 * Test the timing relability of signal delivery by sending alternating
 * USR1 and USR2 signals to a child process, with a specified delay in
 * nanoseconds between signals. The "out of order" count (that is, when
 * two USR1 are seen in a row, for example) for a given delay will
 * depend on the hardware, OS, how busy the OS is, and etc.
 */

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

/* how many signals to process before the child prints stats and exits */
#define MAXITER 99999

struct timespec fork_wait, send_delay;
int prev_sig;
unsigned int ooo_count, sig_count;

void emit_help(void);
void handle(int sig);

int main(int argc, char *argv[])
{
    int ch;
    char *ep;
    pid_t pid;
    long nsecs;

    while ((ch = getopt(argc, argv, "h?")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 1)
        emit_help();

    nsecs = strtol(*argv, &ep, 10);
    if (argv[0][0] == '\0' || *ep != '\0') {
        fprintf(stderr, "could not parse delay nanoseconds\n");
        emit_help();
    }
    if (nsecs < 1 || nsecs > 999999999)
        errx(EX_DATAERR, "delay is out of bounds for nanoseconds\n");

    fork_wait.tv_sec = 1;
    fork_wait.tv_nsec = 0;

    send_delay.tv_sec = 0;
    send_delay.tv_nsec = nsecs;

    pid = fork();
    switch (pid) {
    case -1:
        errx(EX_UNAVAILABLE, "could not vfork");

    case 0:                    /* child */
        signal(SIGUSR1, handle);
        signal(SIGUSR2, handle);
        while (sig_count < MAXITER)
            sleep(1);
        fprintf(stderr, "rate %ld ooo %d sigs %d\n", send_delay.tv_nsec,
                ooo_count, sig_count);
        _exit(0);

    default:                   /* parent */
        signal(SIGCHLD, SIG_IGN);

        /* give child time to fork itself */
        nanosleep(&fork_wait, NULL);

        /*
         * NOTE if there is an error, the child may be left running,
         * as it will only exit after seeing MAXITER signals (would
         * then need to TERM the child, and have a handler for that
         * signal that prints the stats). Otherwise, assuming all goes
         * well, the child will exit after the specified number of
         * signals, which then will case a kill to fail, and the
         * parent to exit.
         */
        while (1) {
            if (kill(pid, SIGUSR1) == -1)
                break;
            nanosleep(&send_delay, NULL);
            if (kill(pid, SIGUSR2) == -1)
                break;
            nanosleep(&send_delay, NULL);
        }
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: sigratetest nsec_delay\n");
    exit(EX_USAGE);
}

void handle(int sig)
{
    sig_count++;
    if (prev_sig == sig)
        ooo_count++;
    else
        prev_sig = sig;
}
