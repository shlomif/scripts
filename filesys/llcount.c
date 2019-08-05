/* llcount - length of input lines */

#include <err.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);
void read_lines(const char *file);

char *Flag_Format = "% 3ld % 5ld % 4ld ";

int main(int argc, char *argv[])
{
    int ch;

#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    while ((ch = getopt(argc, argv, "f:h?x")) != -1) {
        switch (ch) {
        case 'x':
            Flag_Format = "% 3ld %# 5lx % 4ld ";
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

    if (argc > 1)
        emit_help();

    if (argc == 0 || strncmp(*argv, "-", (size_t) 2) == 0) {
        read_lines(NULL);
    } else {
        read_lines(*argv);
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fputs("Usage: llcount [-x] [file|-]\n", stderr);
    exit(EX_USAGE);
}

void read_lines(const char *file)
{
    FILE *fh;
    char *line = NULL;
    size_t linebuflen = 0;
    ssize_t numchars;

    ssize_t linenumber = 1;
    ssize_t seenchars = 0;

    if (file) {
        if ((fh = fopen(file, "r")) == NULL)
            err(EX_IOERR, "could not open '%s'", file);
    } else {
        fh = stdin;
    }

    while ((numchars = getline(&line, &linebuflen, fh)) > 0) {
        printf(Flag_Format, linenumber++, seenchars, numchars);
        fwrite(line, numchars, 1, stdout);
        seenchars += numchars;
    }
    if (ferror(fh))
        err(EX_IOERR, "error reading '%s'", file ? file : "-");
}
