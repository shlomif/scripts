/*
 * Testing purposes?
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;

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

    if (setsid() == -1)
        err(EX_OSERR, "could not setsid()");

    if (execvp(*argv, argv) == -1)
        err(EX_OSERR, "could not exec %s", *argv);

    exit(1);                    /* NOTREACHED */
}

void emit_help(void)
{
    fprintf(stderr, "Usage: setsid cmd [cmdarg ..]\n");
    exit(EX_USAGE);
}
