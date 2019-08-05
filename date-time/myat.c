/* myat - because at(1) does not accept a date format I can remember,
 * and this tool is less annoying and much faster than waiting for a
 * resource hog browser to render some JavaScript infested calendar */

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

/* the strange middle-endian time format at(1) wants */
#define AT_TIMEFORMAT "%H:%M %b %d %Y"
#define TIME_STRING_LEN 20

struct tm when;

char *Flag_Chdir;               /* -C */

char Time_String[TIME_STRING_LEN];

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    struct tm *now;
    time_t epoch_now, epoch_when;

#ifdef __OpenBSD__
    if (pledge("exec stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

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

    /* do not need not-POSIX time handling, so enforce that */
    if (!setlocale(LC_TIME, "POSIX"))
        errx(EX_OSERR, "setlocale(3) POSIX failed ??");
    if (setenv("LC_TIME", "POSIX", 1) != 0)
        err(EX_OSERR, "setenv LC_TIME failed ??");

    if (time(&epoch_now) == (time_t) - 1)
        err(EX_OSERR, "time() failed ??");
    if ((now = localtime(&epoch_now)) == NULL)
        err(EX_OSERR, "localtime() failed ??");

    if (strptime(*argv, "%Y-%m-%d", &when)) {
        /* could be a typo and will alter the date so reject */
        if (when.tm_mday == 0)
            errx(1, "will not accept 0 as day of month");
        if (argc == 2)
            if (!strptime(*++argv, "%H:%M", &when))
                errx(EX_USAGE, "could not parse HH:MM");
    } else if (strptime(*argv, "%H:%M", &when)) {
        when.tm_year = now->tm_year;
        when.tm_mon = now->tm_mon;
        when.tm_mday = now->tm_mday;
    } else {
        /* in contrast to the "garbled time" error from at(1) ... */
        errx(EX_USAGE, "need YYYY-MM-DD or HH:MM as first argument");
    }

    /* OpenBSD at(1) checks for past dates; Mac OS X, not so much */
    if ((epoch_when = mktime(&when)) == (time_t) - 1)
        err(EX_OSERR, "mktime failed");
    /* assume minute granularity on at(1) */
    if (epoch_when - 60 < epoch_now)
        errx(1, "cannot schedule jobs in the past");

    /* yucky TZ with DST need this for isdist? guess-o-matic code */
    when.tm_isdst = -1;

    if (strftime(Time_String, TIME_STRING_LEN - 1, AT_TIMEFORMAT, &when) < 1)
        errx(EX_OSERR, "strftime failed ??");

    if (Flag_Chdir) {
        if (chdir(Flag_Chdir) != 0)
            err(EX_IOERR, "could not chdir to '%s'", Flag_Chdir);
    }

    execlp("at", "at", Time_String, (char *) 0);
    err(EX_OSERR, "exec failed");
}

void emit_help(void)
{
    fputs("Usage: myat [-C dir] YYYY-MM-DD [HH:MM]\n"
            "       myat [-C dir] HH:MM      (assumes today)\n", stderr);
    exit(EX_USAGE);
}
