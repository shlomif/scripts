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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

/* TODO might use _NSIG - 1 but that would need portability research */
#define SIG_MAX 32

void emit_help(void);

int main(int argc, char *argv[])
{
    int advance, ch, sig_num;
    sigset_t block;

    /* C-c by default, unless -s flag */
    sigemptyset(&block);
    sigaddset(&block, SIGINT);

    while ((ch = getopt(argc, argv, "h?s:")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
            emit_help();
            /* NOTREACHED */
        case 's':
            sigemptyset(&block);
            while (sscanf(optarg, "%d%*[^0-9]%n", &sig_num, &advance) == 1) {
                if (sig_num < 1)
                    errx(EX_DATAERR,
                         "signal number must be positive integer");
                else if (sig_num > SIG_MAX)
                    errx(EX_DATAERR,
                         "signal number out of range: %d exceeds max of %d",
                         sig_num, SIG_MAX);
                sigaddset(&block, sig_num);
                optarg += advance;
            }
            break;
        default:
            emit_help();
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
    exit(EXIT_SUCCESS);         /* NOTREACHED */
}

void emit_help(void)
{
    fprintf(stderr, "Usage: blocksig [-s 'signum ..'] command [args ..]\n");
    exit(EX_USAGE);
}
