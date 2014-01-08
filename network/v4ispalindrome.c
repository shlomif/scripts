/*
 * Determines if an IPv4 address is a palindromic address. Such
 * addresses need no special htonl/ntohl handling, regardless of host
 * versus network byte order. There are relatively few of these on
 * the Internet (65536).
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);

bool Flag_Quiet;                /* -q */

int Exit_Status = 1;            /* failed test by default */

int
main(int argc, char *argv[])
{
    int ch, ret;
    struct in_addr ip;

    while ((ch = getopt(argc, argv, "h?q")) != -1) {
        switch (ch) {
        case 'q':
            Flag_Quiet = true;
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

    if ((ret = inet_pton(AF_INET, *argv, &ip)) != 1) {
        if (ret == -1)
            err(EX_DATAERR, "inet_pton() failed");
        else
            errx(EX_DATAERR, "inet_pton() failed");
    }

    /* PORTABILITY will fail on middle-endian systems such as PDP-11,
     * but not too worried about such. Otherwise, this compares the
     * outer bytes and the inner bytes for symmetry - x.y.y.x */
    if ((ip.s_addr & 255) == (ip.s_addr >> 24 & 255) &&
        (ip.s_addr & 255 >> 8) == (ip.s_addr >> 16 & 255)
        ) {
        if (!Flag_Quiet)
            printf("yes\n");
        Exit_Status = EXIT_SUCCESS;
    }

    exit(Exit_Status);
}

void
emit_help(void)
{
    fprintf(stderr, "Usage: [-q] ipv4-address\n");
    exit(EX_USAGE);
}
