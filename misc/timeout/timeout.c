/*
 * Times out the specified command after the specified amount of time.
 * Originally (written in Perl and) motivated by a Sisyphean Java Mess
 * that would not exit itself, but also handy for closing web browsers
 * after some amount of time, or similar needs.
 *
 * Usage:
 *   timeout [-q] -- duration command [command args ...]
 *
 * For example to kill off a browser after some time:
 *   exec timeout -- 11h systrace -a -e xombrero
 *
 * Huh, the Linux tools apparently have a `timeout` thingy. Go figure.
 *
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <err.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

/* exit status to use if timeout occurs */
#define EXIT_TIMEOUT 2

void emit_help(void);
void handle_alarm(int sig);
int parse_duration(char *durp);
int spec2secs(int d, char c);

bool Kill_Bill;

/* global mostly for the free zeroing of all the various struct fields */
struct itimerval iTimer;

bool Flag_Quiet;                /* -q */

int main(int argc, char *argv[])
{
    int ch, exit_status, status;
    pid_t bill;

    exit_status = EXIT_SUCCESS;

    while ((ch = getopt(argc, argv, "h?q")) != -1) {
        switch (ch) {
        case 'q':
            Flag_Quiet = true;
            break;

        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 2)
        emit_help();

    iTimer.it_value.tv_sec = parse_duration(*argv++);

    bill = vfork();
    if (bill == 0) {            /* child */
        if (execvp(*argv, argv) == -1)
            err(EX_OSERR, "could not exec %s", *argv);
        /* NOTREACHED */

    } else if (bill > 0) {      /* parent */
        /* do not restart the wait() after SIGALRM, skip to the end... */
        if (siginterrupt(SIGALRM, 1) == -1)
            err(EX_SOFTWARE, "could not siginterrupt()");

        if (signal(SIGALRM, handle_alarm) == SIG_ERR)
            err(EX_SOFTWARE, "could not setup signal()");

        if (setitimer(ITIMER_REAL, &iTimer, NULL) == -1)
            err(EX_SOFTWARE, "could not setitimer()");

        wait(&status);

        /* the end */
        if (Kill_Bill) {
            if (!Flag_Quiet)
                warnx("duration %ld exceeded: killing pid %d",
                      (long)iTimer.it_value.tv_sec, bill);
            /*
             * Assume child is well behaved, and does not deadlock or
             * otherwise require multiple signals (race condition risk) to
             * take down.
             */
            if (kill(bill, SIGTERM) == -1)
                err(EX_OSERR, "could not kill child pid %d", bill);

            exit_status = EXIT_TIMEOUT;
        } else {
            /*
             * Pass on the child exit status. These can be illustrated
             * via something like:
             *
             *   timeout 99 perl -e 'exit 42'      ; echo $?
             *   timeout 99 perl -e 'kill 15, $$'  ; echo $?
             */
            if (WIFEXITED(status))
                exit_status = WEXITSTATUS(status);
            else if (WIFSIGNALED(status))
                exit_status = 128 + WTERMSIG(status);
        }
    } else {
        err(EX_OSERR, "could not fork()");
    }

    exit(exit_status);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: timeout [-q] -- duration command [cmd args ...]\n");
    exit(EX_USAGE);
}

void handle_alarm(int sig)
{
    Kill_Bill = true;
}

/*
 * Parse the duration, could either be just "42" or "1m3s"
 */
int parse_duration(char *durp)
{
    char duration_spec;
    int advance, ret, sleep_duration, tmp_duration;

    sleep_duration = tmp_duration = 0;

    ret = sscanf(durp, "%9d%n", &tmp_duration, &advance);
    if (ret != 1)
        errx(EX_DATAERR,
             "duration must be a positive integer or short form (e.g. 62 or 1m2s)");
    durp += advance;

    ret = sscanf(durp, "%1c%n", &duration_spec, &advance);
    if (ret == 0 || ret == EOF) {
        /* no following character, so just a raw value */
        sleep_duration = tmp_duration;

    } else if (ret == 1) {
        /* human short form, perhaps */
        durp += advance;
        sleep_duration += spec2secs(tmp_duration, duration_spec);

        while ((ret =
                sscanf(durp, "%9d%1c%n", &tmp_duration, &duration_spec,
                       &advance)) == 2) {
            durp += advance;
            sleep_duration += spec2secs(tmp_duration, duration_spec);
        }

    } else {
        /* should not reach here. curious if do. */
        err(EX_SOFTWARE, "unknown return '%d' while parsing duration", ret);
    }

    if (sleep_duration < 1)
        err(EX_DATAERR, "duration must work out to a positive integer");

    return sleep_duration;
}

/* converts 3, "m" or whatever into an appropriate number of seconds */
int spec2secs(int d, char c)
{
    int new_duration = 0;

    switch (c) {
    case 's':
        new_duration = d;
        break;
    case 'm':
        new_duration = d * 60;
        break;
    case 'h':
        new_duration = d * 3600;
        break;
    case 'd':
        new_duration = d * 86400;
        break;
    case 'w':
        new_duration = d * 604800;
        break;
    default:
        err(EX_DATAERR,
            "unknown duration specification '%c', must be one of 'smhdw'", c);
    }

    return new_duration;
}
