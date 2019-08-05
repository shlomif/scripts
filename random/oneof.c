/* oneof - pick a random argument and display it */

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    uint32_t result;

#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
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

    if (argc == 0)
        emit_help();

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

    /* yes this could have worse modulo bias than coinflip.c but I'm not
     * worried about it given the expected numbers of arguments */
    puts(argv[result % argc]);

    exit(result ? 0 : 1);
}

void emit_help(void)
{
    fputs("Usage: oneof arg [..]\n", stderr);
    exit(EX_USAGE);
}
