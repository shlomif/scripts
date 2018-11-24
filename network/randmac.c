/* randmac - generates random Media Access Control (MAC) addresses */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#include "macutil.h"

/* http://tools.ietf.org/html/rfc2373 - section 2.5.1 - 2 or 0010 for
 * the 2nd octet (index 1) of eui[] */
#define IPV6_FLIP     1U<<1

/* How to toggle the appropriate bits for the flags */
#define MAC_MULTICAST 1U<<0
#define MAC_PRIVATE   1U<<1

bool Flag_IPv6ify;              /* -6 */
bool Flag_Literal;              /* -L */
bool Flag_Multicast;            /* -m */
bool Flag_Private;              /* -p */
size_t mac_bytes = 6;           /* -B or 48 bits if unspecified */

/* default template for generated MAC */
char mac48tmpl[] = "XX:XX:XX:XX:XX:XX";

void emit_usage(void);

int main(int argc, char *argv[])
{
    int ch;
    char *mstrp = mac48tmpl;
    uint8_t *mp;

    while ((ch = getopt(argc, argv, "6B:Lh?mp")) != -1) {
        switch (ch) {
        case '6':
            Flag_IPv6ify = true;
            break;
            /* TODO auto-scan string for X and hexchars to figure out size so can
             * then remove this option. */
        case 'B':
            mac_bytes = (size_t) flagtoul(ch, optarg, 1UL, 64UL);
            break;
        case 'L':
            Flag_Literal = true;
            break;
        case 'm':
            Flag_Multicast = true;
            break;
        case 'p':
            Flag_Private = true;
            break;
        case 'h':
        case '?':
        default:
            emit_usage();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if ((mp = calloc(mac_bytes, sizeof(uint8_t))) == NULL)
        err(EX_OSERR, "calloc failed");

    if (argc > 0)
        mstrp = *argv;

    if (str2mac(mstrp, mp, mac_bytes) == -1)
        errx(EX_DATAERR, "not enough data to fill MAC");

    if (!Flag_Literal) {
        if (Flag_Multicast) {
            *mp |= MAC_MULTICAST;
        } else {
            *mp &= ~(MAC_MULTICAST);
        }
        if (Flag_Private) {
            *mp |= MAC_PRIVATE;
        } else {
            *mp &= ~(MAC_PRIVATE);
        }
    }

    /* Emit the MAC address, possibly as EUI-64 with -6 */
    if (Flag_IPv6ify) {
        if (mac_bytes != 6)
            errx(EX_SOFTWARE, "do not know how to IPv6ify %ld byte MAC",
                 mac_bytes);
        *mp ^= IPV6_FLIP;
        for (size_t i = 0; i < mac_bytes; i++) {
            printf("%02x", *mp++);
            if (mac_bytes == 6 && i == 2)
                printf("ff:fe");
            if (i < mac_bytes - 1 && i % 2 == 1)
                putchar(':');
        }
    } else {
        for (size_t i = 0; i < mac_bytes; i++) {
            printf("%02x", *mp++);
            if (i < mac_bytes - 1)
                putchar(':');
        }
    }
    putchar('\n');

    exit(EXIT_SUCCESS);
}

void emit_usage(void)
{
    errx(EX_USAGE, "[-6] [-B bytes] [[-m] [-p] | [-L]] [00:00:36:XX:XX:XX]");
}
