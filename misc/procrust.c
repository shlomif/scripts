/* procrust - force all input lines to be the given width */

#include <err.h>
#include <limits.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

void emit_help(void);
void mangle_lines(FILE * fh, const char *fname, const unsigned int width,
                  const char fillchar);

int main(int argc, char *argv[])
{
    char ch;
    char fillchar = ' ';
    unsigned int width;
    FILE *fh;

    while ((ch = getopt(argc, argv, "h?f:")) != -1) {
        switch (ch) {
        case 'f':
            fillchar = optarg[0];
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
    width =
        (unsigned int) argtoul("width", *argv, 0U,
                               (unsigned long) INT_MAX - 1) + 1;

    if (argc == 1 || strncmp(argv[1], "-", (size_t) 2) == 0) {
        mangle_lines(stdin, "-", width, fillchar);
    } else {
        argv++;
        while (*argv != NULL) {
            if ((fh = fopen(*argv, "r")) == NULL)
                err(EX_IOERR, "could not open '%s'", *argv);
            mangle_lines(fh, *argv, width, fillchar);
            argv++;
        }
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: procrust [-f fillchar] width [file ..|-]\n");
    exit(EX_USAGE);
}

void mangle_lines(FILE * fh, const char *fname, const unsigned int width,
                  const char fillchar)
{
    char *line = NULL;
    size_t linebuflen = 0;
    ssize_t numchars;

    while ((numchars = getline(&line, &linebuflen, fh)) > 0) {
        if (numchars > width) {
            line[width - 1] = '\n';
            fwrite(line, (size_t) 1, width, stdout);
        } else if (numchars < width) {
            unsigned int delta = width - numchars;
            fwrite(line, (size_t) 1, numchars - 1, stdout);
            for (int i = 0; i < delta; i++)
                putchar(fillchar);
            putchar('\n');
        } else {
            fwrite(line, (size_t) 1, numchars, stdout);
        }
    }
    if (ferror(fh))
        err(EX_IOERR, "error reading '%s'", fname);
}
