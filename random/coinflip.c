/* coinflip - decisions are tough. this service is often provided by IRC
 * bots, but those are not available when one is off of the Internet */

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

int Flag_Quiet;                 /* -q */

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    uint32_t result;

#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    while ((ch = getopt(argc, argv, "h?q")) != -1) {
        switch (ch) {
        case 'q':
            Flag_Quiet = 1;
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

#ifdef __OpenBSD__
    result = arc4random();
#else
    if ((ch = open("/dev/urandom", O_RDONLY)) == -1)
        err(EX_IOERR, "open /dev/urandom failed");
    if (read(ch, &result, sizeof(uint32_t)) != sizeof(uint32_t))
        err(EX_IOERR, "read from /dev/urandom failed");
    /* as this process should not be long for the ps table ... */
    //close(ch);
#endif

    /* yes this has modulo bias but I'm not exactly worried about it */
    result %= 2;

    if (!Flag_Quiet)
        puts(result ? "heads" : "tails");

    exit(result ? 0 : 1);
}

void emit_help(void)
{
    fputs("Usage: coinflip [-q]\n", stderr);
    exit(EX_USAGE);
}
