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
int Return_Value = EXIT_SUCCESS;

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
#ifdef __OpenBSD__
    // since OpenBSD 5.4
    Program_Name = getprogname();
#else
    Program_Name = *argv;
#endif

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
    const char *shortname;
#ifdef __OpenBSD__
    shortname = Program_Name;
#else
    if ((shortname = strrchr(Program_Name, '/')) != NULL)
        shortname++;
    else
        shortname = Program_Name;
#endif

    fprintf(stderr, "Usage: %s TODO\n", shortname);

    exit(EX_USAGE);
}
