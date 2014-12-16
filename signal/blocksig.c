/*
 * Blocks signals then replaces self with the specified command. By
 * default, SIGINT is masked, though with the -s option a list of signal
 * numbers will be masked. Usage:
 *
 *   # these are the same (assuming SIGINT is 2)
 *   blocksig      sleep 30
 *   blocksig -s 2 sleep 30
 *
 *   blocksig -s '1 2 15 30 31' ...    # see signal.h for numbers
 *
 * A useful use of this program is mask C-c from something that
 * otherwise exits the game with no means to restore:
 *
 *   blocksig /usr/games/trek
 */

#include <err.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

/* TODO might use _NSIG - 1 but that would need portability research */
#define MAX_SIG 32

bool Seen_Sig[MAX_SIG];

void emit_help(void);

int main(int argc, char *argv[])
{
    int advance = 0;
    int ch, sig_num;
    sigset_t block;

    /* C-c by default, unless -s flag */
    sigemptyset(&block);
    sigaddset(&block, SIGINT);

    while ((ch = getopt(argc, argv, "h?s:")) != -1) {
        switch (ch) {
        case 's':
            sigemptyset(&block);

            if (!isdigit(*optarg))
                errx(EX_DATAERR, "signals must be plain integers");

            while (sscanf(optarg, "%2d%n", &sig_num, &advance) == 1) {
                if (sig_num < 1)
                    errx(EX_DATAERR, "signal number must be positive integer");
                else if (sig_num > MAX_SIG)
                    errx(EX_DATAERR, "signal number exceeds max of %d",
                         MAX_SIG);

                if (!Seen_Sig[sig_num - 1]) {
                    sigaddset(&block, sig_num);
                    Seen_Sig[sig_num - 1] = true;
                }
                optarg += advance;

                if (*optarg == '\0')
                    break;

                if (*optarg != ' ')
                    errx(EX_DATAERR, "signals must be spaced apart");

                optarg++;
                if (!isdigit(*optarg))
                    errx(EX_DATAERR, "signals must be plain integers");
            }
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

    if (argc < 1)
        emit_help();

    if (sigprocmask(SIG_BLOCK, &block, NULL) == -1)
        err(EX_OSERR, "could not set sigprocmask of %d", block);

    if (execvp(*argv, argv) == -1)
        err(EX_OSERR, "could not exec %s", *argv);

    exit(1);                    /* NOTREACHED */
}

void emit_help(void)
{
    fprintf(stderr, "Usage: blocksig [-s 'signum ..'] command [args ..]\n");
    exit(EX_USAGE);
}
