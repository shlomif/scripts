/*
 * Length of line count thingy for non-binary files.
 */

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define FMT_BUF 42

int main(int argc, char *argv[])
{
    char fmt[FMT_BUF] = "% 3ld % 5ld % 4ld ";
    char *linep;
    FILE *fh;
    int ch;
    size_t len, slen = 0;
    unsigned long lineno = 1;

    while ((ch = getopt(argc, argv, "f:x")) != -1) {
        switch (ch) {
        case 'f':
            /* not really a sensible thing to offer, especially if in
             * any way untrustworthy input is involved, e.g. any human
             * with an input device */
            strncpy(fmt, optarg, FMT_BUF);
            fmt[FMT_BUF - 1] = '\0';
            break;
        case 'x':
            strncpy(fmt, "% 3ld %# 5lx % 4ld ", FMT_BUF);
            fmt[FMT_BUF - 1] = '\0';
            break;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 1)
        errx(EX_USAGE, "[-x | -f ...] file");

    if ((fh = fopen(*argv, "r")) == NULL)
        err(EX_IOERR, "could not open %s", *argv);

    while ((linep = fgetln(fh, &len)) != NULL) {
        dprintf(STDOUT_FILENO, fmt, lineno++, slen, len);
        write(STDOUT_FILENO, linep, len);
        slen += len;
    }
    if (ferror(fh))
        err(EX_IOERR, "error reading file");

    exit(EXIT_SUCCESS);
}
