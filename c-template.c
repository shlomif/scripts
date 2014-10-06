/*
 * Blah blah blah
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

const char *Program_Name;

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    Program_Name = *argv;

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



    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    const char *shortname;
    if ((shortname = strrchr(Program_Name, '/')) != NULL)
        shortname++;
    else
        shortname = Program_Name;

    fprintf(stderr, "Usage: %s TODO\n", shortname);

    exit(EX_USAGE);
}
