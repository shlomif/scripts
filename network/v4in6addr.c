/*
 * Converts the IPv4 address given as the first argument into an IPv6
 * embedded form as per [RFC 6052], for example with the "well known"
 * prefix mentioned in TCP/IP Illustrated, Vol 1, 2nd edition (p.52):
 *
 *   v4in6addr -t 64:ff9b:: 192.0.2.1
 *
 * Requires -std=c99 to compile. Tested (albeit briefly) on both
 * OpenBSD/amd64 and Debian/ppc for little- vs. big-endian needs.
 */

/* ugh, linux */
#if defined(linux) || defined(__linux) || defined(__linux__)
extern char *optarg;
extern int optind, opterr, optopt;
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#define S6ADDR_MAX 16

struct in_addr v4addr;
/* default v6 template and -t flag argument */
struct in6_addr v6addr;

long Flag_Prefix = 96;          /* -p prefix length */

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, ret;

    while ((ch = getopt(argc, argv, "hp:t:")) != -1) {
        switch (ch) {
        case 'p':
            Flag_Prefix = (long) flagtoul(ch, optarg, 0UL, LONG_MAX);
            if (Flag_Prefix < 32 || Flag_Prefix > 96)
                errx(EX_DATAERR, "option -p must be one of 32 40 48 56 64 96");
            break;

        case 't':
            if ((ret = inet_pton(AF_INET6, optarg, &v6addr)) != 1) {
                if (ret == -1)
                    err(EX_DATAERR, "inet_pton() could not parse -t '%s'",
                        optarg);
                else
                    errx(EX_DATAERR, "inet_pton() could not parse -t '%s'",
                         optarg);
            }
            break;

        case 'h':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc == 0)
        emit_help();

    if ((ret = inet_pton(AF_INET, *argv, &v4addr)) != 1) {
        if (ret == -1)
            err(EX_DATAERR, "inet_pton() could not parse '%s'", *argv);
        else
            errx(EX_DATAERR, "inet_pton() could not parse '%s'", *argv);
    }

    /* little endian arch? big endian? other? normalize. */
    v4addr.s_addr = htonl(v4addr.s_addr);

    /* In all cases bits 64-71 must be zero [RFC 4291] */
    v6addr.s6_addr[8] = 0;

    /* figure out what else needs to happen to v6 address */
    switch (Flag_Prefix) {
    case 96:
        /* just copy v4 into ultimate 32 bits - nothing else to zero */
        for (int i = 0; i < 4; i++)
            v6addr.s6_addr[i + 12] = v4addr.s_addr >> ((3 - i) * 8) & 0xff;

        break;
    case 64:
        /* v4 address follows bits 64-71 */
        for (int i = 0; i < 4; i++)
            v6addr.s6_addr[i + 9] = v4addr.s_addr >> ((3 - i) * 8) & 0xff;

        /* zero ultimate 24 bits (reserved suffix) */
        for (int i = 13; i < S6ADDR_MAX; i++)
            v6addr.s6_addr[i] = 0;

        break;
    case 56:
        /* 8 bits prior to bits 64-71, remaining 24 after */
        v6addr.s6_addr[7] = v4addr.s_addr >> 24 & 0xff;
        for (int i = 0; i < 3; i++)
            v6addr.s6_addr[i + 9] = v4addr.s_addr >> ((2 - i) * 8) & 0xff;

        /* zero ultimate 32 bits (reserved suffix) */
        for (int i = 12; i < S6ADDR_MAX; i++)
            v6addr.s6_addr[i] = 0;

        break;
    case 48:
        /* 16 bits prior to bits 64-71, 16 after */
        for (int i = 0; i < 2; i++)
            v6addr.s6_addr[i + 6] = v4addr.s_addr >> ((3 - i) * 8) & 0xff;
        for (int i = 0; i < 2; i++)
            v6addr.s6_addr[i + 9] = v4addr.s_addr >> ((1 - i) * 8) & 0xff;

        /* zero ultimate 40 bits (reserved suffix) */
        for (int i = 11; i < S6ADDR_MAX; i++)
            v6addr.s6_addr[i] = 0;

        break;
    case 40:
        /* 24 bits prior to bits 64-71, remaining 8 after */
        for (int i = 0; i < 3; i++) {
            v6addr.s6_addr[i + 5] = v4addr.s_addr >> ((3 - i) * 8) & 0xff;
        }
        v6addr.s6_addr[9] = v4addr.s_addr & 0xff;

        /* zero ultimate 48 bits (reserved suffix) */
        for (int i = 10; i < S6ADDR_MAX; i++)
            v6addr.s6_addr[i] = 0;

        break;
    case 32:
        /* v4 address follows 32-bit prefix */
        for (int i = 0; i < 4; i++)
            v6addr.s6_addr[i + 4] = v4addr.s_addr >> ((3 - i) * 8) & 0xff;

        /* zero ultimate 56 bits (reserved suffix) */
        for (int i = 9; i < S6ADDR_MAX; i++)
            v6addr.s6_addr[i] = 0;

        break;
    default:
        errx(EX_DATAERR,
             "unknown prefix length, must be one of 32 40 48 56 64 96");
    }

    for (int i = 0; i < S6ADDR_MAX; i++) {
        printf("%02x", v6addr.s6_addr[i]);
        if (i < S6ADDR_MAX - 1 && i % 2 == 1)
            putchar(':');
    }
    putchar('\n');

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: [-p prefixlen] [-t v6addr] ipv4-address\n");
    exit(EX_USAGE);
}
