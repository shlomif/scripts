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

/* for strftime out, minimum sized to the default at time format in my
 * locale, below. */
#define BUF_LEN_MIN 18
#define BUF_LEN_MAX 72

void emit_help(void);

struct tm when;                 /* free zeroing of struct members */

int main(int argc, char *argv[])
{
    int ch;
    /* for strftime output */
    size_t buf_len = BUF_LEN_MIN;
    char *buf;

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

    if (argc < 1)
        emit_help();

    if ((buf = malloc(buf_len)) == NULL)
        err(EX_OSERR, "malloc() failed to create output buffer");

    if (!strptime(*argv, "%Y-%m-%d", &when))
        errx(EX_DATAERR, "could not parse YYYY-MM-DD");
    if (argc == 2) {
        if (!strptime(*++argv, "%H:%M", &when))
            errx(EX_DATAERR, "could not parse HH:MM");
    } else {
        when.tm_hour = when.tm_min = 0;
    }

    while (strftime(buf, buf_len, "%H:%M %b %d %Y", &when) < 1) {
        buf_len <<= 1;
        if (buf_len > BUF_LEN_MAX)
            errx(EX_SOFTWARE, "strftime() output too large for buffer %d",
                 BUF_LEN_MAX);
        if ((buf = realloc(buf, buf_len)) == NULL)
            err(EX_OSERR,
                "realloc() could not resize output buffer to %ld", buf_len);
    }

    if (execlp("at", "at", buf, NULL) == -1)
        err(EX_OSERR, "could not exec at");

    exit(1);                    /* NOTREACHED due to exec or so we hope */
}

void emit_help(void)
{
    fprintf(stderr, "Usage: myat YYYY-MM-DD [HH:MM]\n");
    exit(EX_USAGE);
}
