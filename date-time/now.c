/*
 * Mostly due to date(1) being non-portable and needing an easy way to
 * generate Month Day values for things undergoing fermentation, e.g.
 *
 *   now +5w
 *
 * for a batch of honey mead vinegar.
 */

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#define MINOUT 6
#define MAXOUT 96

const char *ftime_tmpl = "%b %d";

void emit_help(void);

int main(int argc, char *argv[])
{
    char *outbuf, offset_type;
    int ch;
    unsigned int offset;
    size_t count, outbuf_len;
    struct tm *when;
    time_t epoch;

    while ((ch = getopt(argc, argv, "h?")) != -1) {
        switch (ch) {

        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (time(&epoch) == (time_t) - 1)
        err(EX_OSERR, "time() failed");
    if ((when = localtime(&epoch)) == NULL)
        err(EX_OSERR, "localtime() failed");

    /* limit of 2 here mostly because fermentation times rarely exceed a
     * small number of weeks, and also to prevent INT_MAX or such from
     * being entered. */
    if (sscanf(*argv, "%2u%n", &offset, &ch) != 1) {
        warnx("could not parse number of days or weeks");
        emit_help();
    }
    *argv += ch;
    if (sscanf(*argv, "%c", &offset_type) == 1) {

        if (isdigit(offset_type)) {
            warnx("day or week count must be less than 99");
            emit_help();
        }

        switch (offset_type) {
        case 'd':
            break;

        case 'w':
            offset *= 7;
            break;

        default:
            warnx("count must be followed by 'd' or 'w' or nothing");
            emit_help();
        }
    }

    when->tm_mday += offset;

    epoch = mktime(when);
    when = localtime(&epoch);

    outbuf_len = MINOUT;
    if ((outbuf = malloc(outbuf_len)) == NULL)
        err(EX_OSERR, "malloc() failed to create output buffer");

    while ((count = strftime(outbuf, outbuf_len, ftime_tmpl, when)) == 0) {
        outbuf_len <<= 1;
        if (outbuf_len > MAXOUT)
            errx(EX_SOFTWARE, "strftime() output too large for buffer %d",
                 MAXOUT);
        if ((outbuf = realloc(outbuf, outbuf_len)) == NULL)
            err(EX_OSERR,
                "realloc() failed to resize strftime() output to %lu",
                outbuf_len);
    }
    write(STDOUT_FILENO, outbuf, count);
    putchar('\n');

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: now +N[dw]\n");
    exit(EX_USAGE);
}
