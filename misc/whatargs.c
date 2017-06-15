/* What arguments were passed? Similar to `echo ... | hexdump ...` but
 * without the vagaries of how echo(1) may or may not behave. Another
 * option is to run the process being debugged under strace(1) or the
 * like to see exactly what the arguments are. */

#include <err.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);

int main(int argc, char *argv[])
{
    char *c;

    argv++;
    if (*argv == NULL) emit_help();

    while (*argv != NULL) {
        c = *argv;
        while (*c != '\0') {
            printf("%3c", isprint(*c) ? *c : '?');
            c++;
        }
        putchar('\n');

        c = *argv;
        while (*c != '\0') {
            printf("%3x", *c);
            c++;
        }
        putchar('\n');

        argv++;
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: whatargs ...\n");
    exit(EX_USAGE);
}
