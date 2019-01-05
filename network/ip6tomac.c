/* ip6tomac - extracts vendor portion of MAC address from IPv6 address */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

int Flag_FullMac;               /* -f -V */
int Flag_Sep = '-';             /* -s */

void emit_help(void);

int main(int argc, char *argv[])
{
    char **ap;
    int ch;
    int ret = 0;
    struct in6_addr v6addr;

    while ((ch = getopt(argc, argv, "fh?s:V")) != -1) {
        switch (ch) {
        case 'f':
            Flag_FullMac = 1;
            break;
        case 's':
            Flag_Sep = *optarg;
            break;
        case 'V':
            Flag_FullMac = 0;
            break;
        case 'h':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc == 0 || *argv == NULL)
        emit_help();

    /* nix any %blah interface bit that might be riding in e.g. on a
     * fe80 copy/paste from ifconfig (as that bit screws up inet_pton()) */
    ap = argv;
    *ap = strsep(argv, "%");

    if ((ret = inet_pton(AF_INET6, *ap, &v6addr)) != 1) {
        if (ret == -1)
            err(EX_OSERR, "system error parsing IPv6 address");
        else
            errx(EX_DATAERR, "could not parse IPv6 address (%d) ??", ret);
    }

    /* only if the the ff:fe bits are set... */
    if (v6addr.s6_addr[11] == 255 && v6addr.s6_addr[12] == 254) {
        /* twiddle global/local bit */
        v6addr.s6_addr[8] ^= 1 << 1;
        if (Flag_FullMac) {
            printf("%02X%c%02X%c%02X%c%02X%c%02X%c%02X\n", v6addr.s6_addr[8],
                   Flag_Sep, v6addr.s6_addr[9], Flag_Sep, v6addr.s6_addr[10],
                   Flag_Sep, v6addr.s6_addr[13], Flag_Sep, v6addr.s6_addr[14],
                   Flag_Sep, v6addr.s6_addr[15]
                );
        } else {
            printf("%02X%c%02X%c%02X\n", v6addr.s6_addr[8], Flag_Sep,
                   v6addr.s6_addr[9], Flag_Sep, v6addr.s6_addr[10]);
        }
    } else {
        exit(1);
    }
    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: ip6tomac [-f | -V] [-s sep] ipv6-address\n");
    exit(EX_USAGE);
}
