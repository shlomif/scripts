/* nomfd - opens a lot of file descriptors */

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

long Flag_FD_Max;               /* -f */

void emit_usage(void);

int main(int argc, char *argv[])
{
    int ch;

#ifdef __OpenBSD__
    if (pledge("rpath stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    while ((ch = getopt(argc, argv, "f:h?")) != -1) {
        switch (ch) {
        case 'f':
            Flag_FD_Max = (long) flagtoul(ch, optarg, 4UL, LONG_MAX);
            break;
        case 'h':
        case '?':
        default:
            emit_usage();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 1)
        emit_usage();

    if (Flag_FD_Max == 0)
        Flag_FD_Max = LONG_MAX;

    if (Flag_FD_Max >= sysconf(_SC_OPEN_MAX))
        warnx("fd count %ld in excess of _SC_OPEN_MAX %ld",
              Flag_FD_Max, sysconf(_SC_OPEN_MAX));

    /* start at three because of std{in,out,err} */
    for (long i = 3; i < Flag_FD_Max; i++) {
        if (open(*argv, O_RDONLY) < 0) {
            warn("open failed at %ld of %ld", i, Flag_FD_Max);
            break;
        }
    }

    /* presumably this is where file I/O or other things in other
     * processes are done, C-c or otherwise signal to abort */
    while (1)
        sleep(UINT_MAX);

    exit(EXIT_SUCCESS);         /* NOTREACHED */
}

void emit_usage(void)
{
    fputs("Usage: nomfd [-f fdcount] file\n", stderr);
    exit(EX_USAGE);
}
