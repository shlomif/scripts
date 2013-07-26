/*
 * Blah blah blah
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
            emit_help();
            /* NOTREACHED */
        default:
            emit_help();
        }
    }
    argc -= optind;
    argv += optind;



    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: TODO\n");
    exit(EX_USAGE);
}
