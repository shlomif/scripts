/*
 * Opens potentially a lot of file descriptors on the specified file. Handy for
 * who knows what purpose, perhaps checking how write speeds change with this
 * many FD then open on the file...
 *
 * See also fuser(1) or lsof(8) to help diagnose situations where many open
 * file descriptors are suspected.
 */

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

unsigned int Flag_FD_Max;       /* -f */

void emit_usage(void);

int main(int argc, char *argv[])
{
    int ch;

    while ((ch = getopt(argc, argv, "f:h?")) != -1) {
        switch (ch) {
        case 'f':
            if (sscanf(optarg, "%u", &Flag_FD_Max) != 1)
                errx(EX_DATAERR, "could not parse -f fd count flag");
            if (Flag_FD_Max < 1)
                errx(EX_DATAERR, "need positive integer for -f fd count");
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
        Flag_FD_Max = INT_MAX;

    if (Flag_FD_Max >= sysconf(_SC_OPEN_MAX))
        warnx("notice: fd count %d in excess of _SC_OPEN_MAX %ld",
              Flag_FD_Max, sysconf(_SC_OPEN_MAX));

    /* start at three because of std{in,out,err} */
    for (unsigned int i = 3; i < Flag_FD_Max; i++) {
        if (open(*argv, O_RDONLY) < 0) {
            warn("open() failed at %d of %d, ceasing opens", i, Flag_FD_Max);
            break;
        }
    }

    /* Presumably this is where file I/O or other things in other processes
     * are done, C-c or otherwise signal to abort this program. */
    while (1)
        sleep(UINT_MAX);

    exit(EXIT_SUCCESS);         /* NOTREACHED */
}

void emit_usage(void)
{
    fprintf(stderr, "Usage: nomfd [-f fdcount] file\n");
    exit(EX_USAGE);
}
