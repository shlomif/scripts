/*
 * Blah de blah
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

int Return_Value = EXIT_SUCCESS;

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



    exit(Return_Value);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: %s TODO\n", getprogname());
    exit(EX_USAGE);
}
