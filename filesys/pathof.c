/* pathof - return path to a file */

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

char buf[PATH_MAX + 1];

void emit_help(void);

int main(int argc, char *argv[])
{
    if (argc != 2)
        emit_help();
    if (realpath(argv[1], (char *) &buf) == NULL)
        err(EX_IOERR, "realpath failed on '%s'", argv[1]);
    puts(buf);
    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: pathof filename\n");
    exit(EX_USAGE);
}
