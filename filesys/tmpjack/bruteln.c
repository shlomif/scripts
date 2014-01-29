/*
 * Tries (a lot) to make a symlink, mainly as test for feasibility of /tmp
 * symlink exploits, which would not be a concern if a) folks put their PID
 * files or whatnot in a tmp dir or b) instead used mktemp(3) properly.
 */

#include <sys/time.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#define USEC_IN_SEC 1000000

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    struct timeval before, after;
    double delta_t;

    while ((ch = getopt(argc, argv, "h?s")) != -1) {
        switch (ch) {
        case 's':              /* noop for compat with ln -s */
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

    gettimeofday(&before, NULL);

    while (1) {
        /* could doubtless be much improved on, for example with inode change
         * notification services, or more intelligent unlink/symlink cycles, or
         * other threads or processes eating up CPU time, etc. */
        symlink(argv[0], argv[1]);
        unlink(argv[1]);
    }

    gettimeofday(&after, NULL);
    delta_t = after.tv_sec - before.tv_sec;
    delta_t += (after.tv_usec - before.tv_usec) / USEC_IN_SEC;
    fprintf(stderr, "delta %.3f\n", delta_t);

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: bruteln existing symlink\n");
    exit(EX_USAGE);
}
