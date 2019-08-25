/* xor - perform xor operations on the input (less typing than perl -E \
 * 'say N ^ M') */

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

long long Flag_Mask = ULLONG_MAX;       /* -M */
int Flag_Unsigned;              /* -u */

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    long long x, y;

#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    while ((ch = getopt(argc, argv, "h?M:u")) != -1) {
        switch (ch) {
        case 'M':
            Flag_Mask = flagtoll(ch, optarg, LLONG_MIN, LLONG_MAX);
            break;
        case 'u':
            Flag_Unsigned = 1;
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
    if (argc != 2)
        emit_help();

    x = argtoll("n1", argv[0], LLONG_MIN, LLONG_MAX);
    y = argtoll("n2", argv[1], LLONG_MIN, LLONG_MAX);

    if (Flag_Unsigned)
        printf("%llu\n", (x ^ y) & Flag_Mask);
    else
        printf("%lld\n", x ^ y);

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fputs("Usage: xor [-u [-M mask]] -- x y\n", stderr);
    exit(EX_USAGE);
}
