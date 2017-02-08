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

    while ((ch = getopt(argc, argv, "h?s")) != -1) {
        switch (ch) {
        case 's':              /* noop for compat with ln -s */
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

    while (1) {
        /* could doubtless be much improved on, for example with inode change
         * notification services, or more intelligent unlink/symlink cycles, or
         * other threads or processes eating up CPU time, etc. */
        symlink(argv[0], argv[1]);
        unlink(argv[1]);
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: bruteln existing symlink\n");
    exit(EX_USAGE);
}
