/*
 * Generates a (possibly) random Media Access Control (MAC) address.
 *
 * https://en.wikipedia.org/wiki/MAC_address
 * http://standards.ieee.org/develop/regauth/oui/oui.txt
 *
 * The -p flag sets the private MAC bit (very unlikely to be assigned by
 * IEEE, but see below) and the -m flag sets the multicast MAC bit. IPv4
 * and IPv6 set certain MAC prefixes (again, see below) for their
 * multicast traffic. NOTE lacking the -m or -p flags, the default is to
 * generate a public, non-multicast address. Specify the -L flag to
 * enforce a literal or locked MAC address. This disables the -m and -p
 * related bit flips.
 *
 * The -B bytes flag and a corresponding number of hex-or-X characters
 * as the first argument to this program allow for other sizes of MAC
 * addresses to be generated.
 *
 * The -6 flag IPv6ifies (EUI-64) a 48-bit MAC address, and will flip a
 * bit, independent of the -L flag. However, the -L flag might be handy
 * with -6 to avoid the otherwise mandatory -m or -p related flips.
 *
 * Incomplete segments may be specified ("01:02:c:03:04:05") and will be
 * converted to the equivalent of "0c" and not to "c0".
 *
 * Example usage:
 *
 *   $ randmac                        # public random 48-bit MAC
 *   $ randmac -p                     # private random 48-bit MAC
 *   $ randmac -p XX:12:34:56:78:XX   # same, but less random
 *   $ randmac -L 40:a6:d9:XX:XX:XX   # Apple, Inc.
 *   $ randmac -L 42:a6:d9:XX:XX:XX   # private
 *   $ randmac -L a:b:c:d:e:f         # not random at all
 *
 *   $ randmac -B 8 -p XX:XX:XX:XX:XX:XX:XX:XX   # private MAC-64
 *
 *   $ randmac -L6 00:11:22:XX:XX:XX | sed 's/^/fe80::/' # IPv6 link-local
 *
 * Prefix material may throw off calculations for the -m and -p options.
 * Use sed(1) afterwards to workaround this limitation, for example to
 * generate a random filename for use under the pxelinux.cfg directory:
 *
 *   $ randmac -p XX-XX-XX-XX-XX-XX | sed 's/^/01-/'
 *
 * A digression on bits, the contents of out.txt as provided by IEEE,
 * and IP multicast specifics follows.
 *
 * Viewing the bits for a given hex value is possible via:
 *
 *   $ perl -e 'printf "%08b\n", 0x40'
 *   01000000
 *   $ perl -e 'printf "%08b\n", 0x42'
 *   01000010
 *
 * So 42:a6:d9, being private, enables the penultimate bit, while
 * 40:a6:d9, being assigned to Apple, does not. However! There appear
 * to be several OUI that set the priate or broadcast bits:
 *
 *   $ < oui.txt perl -ne '$s{$1}++ if m/^\s+(..)-.*\(hex/;' \
 *     -e 'END { printf "%08b %s\n", hex($_), $_ for sort keys %s }' \
 *     | perl -nle 'print if m/1. / or m/1 /'
 *   00000010 02
 *   00010001 11
 *   10101010 AA
 *
 * These perhaps should be avoided in "private" assignments, or I've
 * made a mistake somewhere in my calculations. The 11 (broadcast bit
 * enabled) is a single assignment:
 *
 *   11-00-AA   (hex)           PRIVATE
 *
 * While the 02 (various companies) and AA (DEC) show 18 globally
 * assigned prefixes in the private address space. Doubtless the odds of
 * a random private address landing in one of these subnets and
 * conflicting with some actual piece of hardware would be somewhat low.
 *
 * IPv4 broadcast sets all ones (low odds of randomly rolling if given
 * all XXs); IPv4 multicast uses a prefix of 01:00:5e, and IPv6
 * multicast the prefix 33:33. This tool at present takes no effort not
 * to generate something inside these various ranges.
 */

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#include "macutil.h"

/* http://tools.ietf.org/html/rfc2373 - section 2.5.1 - 2 or 0010 for
 * the 2nd byte (index 1) of eui[] */
#define IPV6_FLIP     1U<<1

/* How to toggle the appropriate bits for the flags */
#define MAC_MULTICAST 1U<<0
#define MAC_PRIVATE   1U<<1

void emit_usage(void);

bool Flag_IPv6ify;              /* -6 */
bool Flag_Literal;              /* -L */
bool Flag_Multicast;            /* -m */
bool Flag_Private;              /* -p */
size_t mac_bytes = 6;           /* -B or 48 bits if unspecified */

/* default template for generated MAC */
char mac48tmpl[] = "XX:XX:XX:XX:XX:XX";

int
main(int argc, char *argv[])
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
            if (sscanf(optarg, "%lu", &mac_bytes) != 1)
                errx(EX_DATAERR, "could not parse -B bytes flag");
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

    /* sanity limit; hopefully I'll be retired or out of computing */
    if (mac_bytes >= 64)
        errx(EX_DATAERR, "too many bytes for my blood");

    mp = calloc(mac_bytes, sizeof(uint8_t));
    if (!mp)
        err(EX_UNAVAILABLE, "could not allocate memory for MAC");

    if (argc > 0)
        mstrp = *argv;

    if (str2mac(mstrp, mp, mac_bytes) == -1)
        errx(EX_DATAERR, "not enough data to fill MAC");

    if (!Flag_Literal) {
        if (Flag_Multicast)
            *mp |= MAC_MULTICAST;
        else
            *mp &= ~(MAC_MULTICAST);

        if (Flag_Private)
            *mp |= MAC_PRIVATE;
        else
            *mp &= ~(MAC_PRIVATE);
    }

    /* Emit the MAC address, possibly as EUI-64 with -6 */
    if (Flag_IPv6ify) {
        if (mac_bytes != 6)
            errx(EX_SOFTWARE, "do not know how to IPv6ify %ld byte MAC", mac_bytes);

        *mp ^= IPV6_FLIP;

        for (size_t i = 0; i < mac_bytes; i++) {
            printf("%02x", *mp++);
            if (mac_bytes == 6 && i == 2)
                printf("ff:fe");
            if (i < mac_bytes - 1 && i % 2 == 1)
                putchar(':');
        }
    } else {
        /* regular MAC format */
        for (size_t i = 0; i < mac_bytes; i++) {
            printf("%02x", *mp++);
            if (i < mac_bytes - 1)
                putchar(':');
        }
    }
    putchar('\n');

    exit(EXIT_SUCCESS);
}

void
emit_usage(void)
{
    errx(EX_USAGE, "[-6] [-B bytes] [[-m] [-p] | [-L]] [02:03:04:XX:XX:XX]");
}
