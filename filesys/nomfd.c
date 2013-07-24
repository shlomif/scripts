/*
 * Opens potentially a lot of file descriptors on the specified file.
 * Handy for who knows what purpose, perhaps checking how write speeds
 * change with this many FD then open on the file...
 *
 * See also fuser(1) or lsof(8) to help diagnose situations where many
 * open file descriptors are suspected.
 */

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

unsigned int Flag_FD_Max;                                  /* -f */

void emit_usage(void);

int main(int argc, char *argv[])
{
    int ch;
    unsigned int i;

    while ((ch = getopt(argc, argv, "f:h?")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
            emit_usage();
            /* NOTREACHED */
        case 'f':
            if (sscanf(optarg, "%u", &Flag_FD_Max) != 1)
                errx(EX_DATAERR, "could not parse -f fd count option");
            if (Flag_FD_Max < 1)
                errx(EX_DATAERR, "need positive integer for -f fd count");
            if (Flag_FD_Max >= sysconf(_SC_OPEN_MAX))
                warnx("notice: fd count %d in excess of _SC_OPEN_MAX %ld",
                      Flag_FD_Max, sysconf(_SC_OPEN_MAX));
            break;
        default:
            emit_usage();
        }
    }
    argc -= optind;
    argv += optind;

    if (Flag_FD_Max == 0 || argc != 1)
        emit_usage();

    /* two more opens, free! */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);

    for (i = 0; i < Flag_FD_Max; i++) {
        if (open(*argv, O_RDONLY) < 0) {
            warn("open() failed at %d of %d, ceasing opens", i, Flag_FD_Max);
            break;
        }
    }

    /* Presumably this is where file I/O or other things in other processes
     * are done, C-c or otherwise signal to abort this program. */
    while (1)
        sleep(UINT_MAX);
}

void emit_usage(void)
{
    fprintf(stderr, "Usage: [-f fdcount] file\n");
    exit(EX_USAGE);
}
