/* setsid - mostly handy to disassociate mupdf from vim session so
 * backgrounding vim doesn't then stall out the mupdf display */

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

enum { NOPE = -1, CHILD };

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    pid_t pid;

#ifdef __OpenBSD__
    if (pledge("exec proc stdio", NULL) == -1)
        err(1, "pledge failed");
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

    if ((pid = fork()) == NOPE)
        err(EX_OSERR, "fork failed");

    if (pid == CHILD) {
        if (setsid() == NOPE)
            err(EX_OSERR, "setsid failed");
        execvp(*argv, argv);
        err(EX_OSERR, "could not exec %s", *argv);
    } else {
        exit(0);
    }

    exit(1);                    /* NOTREACHED */
}

void emit_help(void)
{
    fputs("Usage: setsid cmd [cmdarg ..]\n", stderr);
    exit(EX_USAGE);
}
