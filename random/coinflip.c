/*
 * Decisions are tough.
 */

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define coinflip() arc4random() % 2

bool Flag_Quiet;                // -q

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    uint32_t result;

    while ((ch = getopt(argc, argv, "h?q")) != -1) {
        switch (ch) {

        case 'q':
            Flag_Quiet = true;
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

    result = coinflip();

    if (!Flag_Quiet)
        printf("%s\n", result ? "heads" : "tails");

    exit(result ? EXIT_SUCCESS : 1);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: %s\n", getprogname());
    exit(EX_USAGE);
}
