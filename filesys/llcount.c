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

#define INPUT_FORMAT_MAXLEN 99

void emit_help(void);

int main(int argc, char *argv[])
{
    char *fmt = "% 3ld % 5ld % 4ld ";
    char *linep;
    FILE *fh;
    int ch;
    size_t len, slen = 0;
    ssize_t lineno = 1;

    while ((ch = getopt(argc, argv, "f:h?x")) != -1) {
        switch (ch) {
        case 'f':
            /* Not really a sensible thing to offer, especially if in any way
             * untrustworthy input is involved, that is, any human with any
             * input device. */
            if (asprintf(&fmt, "%*s", INPUT_FORMAT_MAXLEN, optarg) == -1)
	      err(EX_SOFTWARE, "asprintf(3) could not copy -f flag");
            break;

        case 'x':
            asprintf(&fmt, "%% 3ld %%# 5lx %% 4ld ");
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

    if (argc != 1)
        emit_help();

    if ((fh = fopen(*argv, "r")) == NULL)
        err(EX_IOERR, "could not open '%s'", *argv);

    while ((linep = fgetln(fh, &len)) != NULL) {
        dprintf(STDOUT_FILENO, fmt, lineno++, slen, len);
        write(STDOUT_FILENO, linep, len);
        slen += len;
    }
    if (ferror(fh))
        err(EX_IOERR, "error reading file");

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: llcount [-x | -f ...] file\n");
    exit(EX_USAGE);
}
