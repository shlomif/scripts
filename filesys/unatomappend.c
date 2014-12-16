/*
 * Test whether can cause corruption by not using O_APPEND for writes. Must be
 * run with at least two processes writing to the same file.
 *
 * Boy howdy! Without append, output file shows definite truncated data on Mac
 * OS X 10.6, with O_APPEND, no corruption. Longer output string always
 * truncated, while shorter 'bbb' pattern never truncated:
 *
 *   ./unatomappend test.u bbb 1000 & ; ./unatomappend test.u aaaaaaaa 1000 &
 *
 * On a busy system, more iterations were necessary to see corruption, perhaps
 * due to the other processes eating up the CPU giving these two test processes
 * fewer chances of interleaving their operations.
 */

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#define INIT_TIME 2             /* Seconds to wait before starting tests */
#define INIT_SLEEP 50           /* ms to sleep while waiting for INIT_TIME */

/* Command line options */
bool Aflag;                     /* use O_APPEND if set */

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, fd;
    long n;
    unsigned long patlen, iters, i, written = 0;
    char *ep;
    struct timespec rqtp;
    time_t starttime;

    while ((ch = getopt(argc, argv, "Ah?")) != -1) {
        switch (ch) {
        case 'A':
            Aflag = true;
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

    if (argv[0] == NULL || argv[0][0] == '\0' || argv[1] == NULL
        || argv[1][0] == '\0' || argv[2] == NULL || argv[2][0] == '\0')
        emit_help();

    if ((fd =
         open(argv[0], O_WRONLY | O_CREAT | (Aflag == 1 ? O_APPEND : 0),
              0644)) < 0)
        err(EX_IOERR, "could not open '%s'", argv[0]);

    patlen = strlen(argv[1]);

    errno = 0;
    iters = strtoul(argv[2], &ep, 10);
    if (argv[2][0] == '\0' || *ep != '\0')
        errx(EX_USAGE, "iterations not a number");
    if ((errno == ERANGE && iters == ULONG_MAX) || iters == 0)
        errx(EX_USAGE, "iterations out of range");

    /* Timing to better line up different runs so that one process is not still
     * doing init stuff while the other is already done writing. Will likely
     * also need to increase the number of iterations on faster hardware. */
    starttime = time(NULL) + INIT_TIME;
    rqtp.tv_sec = 0;
    rqtp.tv_nsec = INIT_SLEEP * (1000 * 1000);
    while (time(NULL) < starttime) {
        nanosleep(&rqtp, NULL);
    }

    for (i = 0; i < iters; i++) {
        if ((lseek(fd, (size_t) 0, SEEK_END)) == -1)
            err(EX_IOERR, "could not seek");

        /* NOTE should be race condition here - another way to test would be to
         * lseek again, and if get a different offset than prior, would have
         * done something naughty to the file? */
        n = write(fd, argv[1], patlen);
        if (n == -1)
            err(EX_IOERR, "write error");
        else if ((unsigned long) n < patlen)
            errx(EX_IOERR, "incomplete write %ld", n);

        written += (unsigned long) n;
    }
    warnx("written '%s' to '%s' total chars %ld", argv[1], argv[0], written);

    if (close(fd) == -1)
        err(EX_IOERR, "close error");

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: unatomappend [-A] file pattern iterations\n");
    exit(EX_USAGE);
}
