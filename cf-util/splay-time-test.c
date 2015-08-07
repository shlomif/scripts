/*
 * Based on code at http://cfwiki.org/cfwiki/index.php/SplayTime_testing
 */

#include <stdio.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#define CF_MACROALPHABET 61     /* a-z, A-Z plus a bit */
#define CF_HASHTABLESIZE 4969   /* prime number */

/* equivalent to SplayTime = ( N ) */
unsigned long Flag_SplayTime = 1;       // -n

void emit_help(void);
int str2hash(char *name);

int main(int argc, char *argv[])
{
    int ch, hash;

    while ((ch = getopt(argc, argv, "h?n:")) != -1) {
        switch (ch) {

        case 'n':
            Flag_SplayTime = flagtoul(ch, optarg, 0UL, (unsigned long) INT_MAX);
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

    while (*argv) {
        hash = str2hash(*argv);
        printf("host=%s hash=%d time=%d\n", *argv, hash,
               (int) Flag_SplayTime * 60 * hash / CF_HASHTABLESIZE);
        argv++;
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: splay-time-test [-n int] host1 [host2 ..]\n");
    exit(EX_USAGE);
}

int str2hash(char *name)
{
    int slot = 0;

    for (int i = 0; name[i] != '\0'; i++) {
        slot = (CF_MACROALPHABET * slot + name[i]) % CF_HASHTABLESIZE;
    }

    return slot;
}
