/* 
 * (Tries to) extract the vendor portion of a MAC from a IPv6 address,
 * which in theory can then be grepped for in the oui.txt registration
 * database, or at least via the vendor bit of said MAC, assuming there
 * is even one embedded.
 *
 * Usage: obtain:
 *   http://standards.ieee.org/develop/regauth/oui/oui.txt
 *
 * then:
 *
 *   grep `ip6tomac fe80::200:aaff:fe01:0203` oui.txt
 *
 * which claims to be:
 *     00-00-AA   (hex)              XEROX CORPORATION
 *
 * though bear in mind some folks do forge or randomize their MAC.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);

int main(int argc, char *argv[])
{
    char **ap;
    int ch;
    int ret = 0;
    struct in6_addr v6addr;

    while ((ch = getopt(argc, argv, "h")) != -1) {
        switch (ch) {
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

    // nix any %blah interface bit that might be riding in e.g. on a fe80
    // copy/paste from ifconfig (as that bit screws up inet_pton())
    ap = argv;
    *ap = strsep(argv, "%");

    if ((ret = inet_pton(AF_INET6, *ap, &v6addr)) != 1) {
        if (ret == -1)
            err(EX_OSERR, "system error parsing IPv6 address");
        else
            errx(EX_DATAERR, "could not parse IPv6 address");
    }
    // only if the the ff:fe bits are set...
    if (v6addr.s6_addr[11] == 255 && v6addr.s6_addr[12] == 254) {
        // twiddle global/local bit
        v6addr.s6_addr[8] ^= 1 << 1;
        printf("%02X-%02X-%02X\n", v6addr.s6_addr[8], v6addr.s6_addr[9],
               v6addr.s6_addr[10]);
    } else {
        exit(1);
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: ip6tomac ipv6-address\n");
    exit(EX_USAGE);
}
