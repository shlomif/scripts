/* Converts YYYY-MM-DD HH:MM:SS into epoch time */

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

void emit_help(void);

bool Flag_Quiet;                /* -q */

/* free zeroing of struct members though creates mday gotcha */
struct tm when;

int main(int argc, char *argv[])
{
    int ch;

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

    if (argc < 1)
        emit_help();

    /* Edge case. If YYYY-MM or YYYY only thing supplied, a mday of 0 (the
     * default due to where declared) will drag the epoch surprisingly back
     * to the previous month. */
    when.tm_mday = 1;

    if (!strptime(*argv, "%Y-%m-%d", &when))
        if (!strptime(*argv, "%Y-%m", &when))
            if (!strptime(*argv, "%Y", &when))
                errx(EX_DATAERR, "could not parse YYYY-MM-DD");
    if (argc > 1) {
        ++argv;
        if (!strptime(*argv, "%H:%M:%S", &when))
            if (!strptime(*argv, "%H:%M", &when))
                if (!strptime(*argv, "%H", &when))
                    errx(EX_DATAERR, "could not parse HH:MM[:SS]");
    }
    if (argc > 2 && !Flag_Quiet)
        warnx("notice: set timezone via TZ env, not after the timestamp");

    tzset();
    printf("%ld\n", (long) timelocal(&when));

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: [env TZ=...] date2epoch [-q] YYYY-MM-DD [HH[:MM[:SS]]]\n");
    exit(EX_USAGE);
}
