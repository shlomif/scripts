/* Mostly handy to disassociate mupdf from vim session so backgrounding
 * vim doesn't then stall out the mupdf display. */

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    pid_t pid;

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

    pid = fork();

    if (pid == 0) {             /* child */
        if (setsid() == -1)
            err(EX_OSERR, "could not setsid()");
        if (execvp(*argv, argv) == -1)
            err(EX_OSERR, "could not exec %s", *argv);

    } else if (pid > 0) {       /* parent */
        exit(0);
    } else {
        err(EX_OSERR, "could not fork()");
    }

    exit(1);                    /* NOTREACHED */
}

void emit_help(void)
{
    fprintf(stderr, "Usage: setsid cmd [cmdarg ..]\n");
    exit(EX_USAGE);
}
