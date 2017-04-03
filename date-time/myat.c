/*
 * Due to at(1) accepting a variety of curious date formats none of
 * which I otherwise use and reading the manual page for at(1) every
 * time I need to use it isn't fun. (Figuring out the strftime(3) magic
 * to avoid the 'garbled time' error from at(1) was annoying enough, but
 * less annoying than using some JavaScript app on Google.)
 */

#include <err.h>
#include <getopt.h>
#include <limits.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

/* Oh, the silly olden days with their middle-endian Month Day Year--no
 * wonder I cannot memorize this! */
#define AT_TIMEFORMAT "%H:%M %b %d %Y"
#define TIME_STRING_LEN 20
char Time_String[TIME_STRING_LEN];

char *Flag_Chdir;               // -C

void emit_help(void);

/* free zeroing of struct members (though creates mday gotcha (that is not
 * relevant here as %d must be supplied by caller, and if they want to set that
 * to 0, well, more power to them)) */
struct tm when;

int main(int argc, char *argv[])
{
    int ch;

    while ((ch = getopt(argc, argv, "C:h?")) != -1) {
        switch (ch) {
        case 'C':
            Flag_Chdir = optarg;
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

    if (argc < 1 || argc > 2)
        emit_help();

    // do not need not-POSIX time handling, so enforce that for this and
    // the subsequent program
    if (!setlocale(LC_TIME, "POSIX"))
        errx(EX_OSERR, "setlocale(3) POSIX failed??");
    if (setenv("LC_TIME", "POSIX", 1) != 0)
        err(EX_OSERR, "setenv LC_TIME failed??");

    if (strptime(*argv, "%Y-%m-%d", &when)) {
        if (argc == 2)
            if (!strptime(*++argv, "%H:%M", &when))
                errx(EX_USAGE, "could not parse HH:MM");
    } else if (strptime(*argv, "%H:%M", &when)) {
        time_t epoch;
        struct tm *now;
        if (time(&epoch) == (time_t) - 1)
            err(EX_OSERR, "time() failed??");
        if ((now = localtime(&epoch)) == NULL)
            err(EX_OSERR, "localtime() failed??");
        when.tm_year = now->tm_year;
        when.tm_mon = now->tm_mon;
        when.tm_mday = now->tm_mday;
    } else {
        /* in contrast to the "garbled time" error from at(1) ... */
        errx(EX_USAGE, "need YYYY-MM-DD or HH:MM as first argument");
    }

    if (strftime(Time_String, TIME_STRING_LEN - 1, AT_TIMEFORMAT, &when) < 1)
        errx(EX_OSERR, "strftime failed??");

    if (Flag_Chdir) {
        if (chdir(Flag_Chdir) != 0)
            err(EX_IOERR, "could not chdir '%s'", Flag_Chdir);
    }

    execlp("at", "at", Time_String, (char *) 0);

    // exec should not return if all proceeds according to plan
    err(EX_OSERR, "could not exec??");
    exit(EX_OSERR);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: myat [-C dir] YYYY-MM-DD [HH:MM]\n");
    fprintf(stderr, "       myat [-C dir] HH:MM      (assumes today)\n");
    exit(EX_USAGE);
}
