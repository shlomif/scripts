/*
 * Due to at(1) accepting a variety of curious date formats none of
 * which I otherwise use and reading the manual page for at(1) every
 * time I need to use it isn't fun. (Figuring out the strftime(3) magic
 * to avoid the 'garbled time' error from at(1) was annoying enough, but
 * less annoying than using a webform for some Google calendar.)
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#define BUF_MAX 42

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    struct tm when;
    char strtime[BUF_MAX];

    while ((ch = getopt(argc, argv, "h?")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
            emit_help();
            /* NOTREACHED */
        default:
            emit_help();
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 1)
        emit_help();

    if (!strptime(*argv, "%Y-%m-%d", &when))
        errx(EX_DATAERR, "could not parse YYYY-MM-DD");
    if (argc == 2) {
        if (!strptime(*++argv, "%H:%M", &when))
            errx(EX_DATAERR, "could not parse HH:MM");
    } else {
        when.tm_hour = when.tm_min = 0;
    }

    if (strftime(strtime, BUF_MAX, "%H:%M %b %d %Y", &when) < 1)
        errx(EX_SOFTWARE, "could not strftime input");

    if (execlp("at", "at", strtime, NULL) == -1)
        err(EX_OSERR, "could not exec at");
    exit(EXIT_SUCCESS);         /* NOTREACHED */
}

void emit_help(void)
{
    fprintf(stderr, "Usage: myat YYYY-MM-DD [HH:MM]\n");
    exit(EX_USAGE);
}
