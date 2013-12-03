/*
 * Generates random MAC-48 address (or other X to random hex conversions).
 * 
 * https://en.wikipedia.org/wiki/MAC_address
 * http://standards.ieee.org/develop/regauth/oui/oui.txt
 *
 * Use the -p option for a private MAC, and -m for a multicast MAC, but
 * these only apply if the initial octet is XX. That is, if the first
 * octet is XX, the -m and -p options will enable the appropriate bits,
 * which otherwise will be set to 0 by default. If the first octet is
 * specified (that is, is not XX to be filled in), the options or
 * absence thereof influence nothing.
 *
 * Example usage:
 *
 *   $ randmac -p
 *   $ randmac -p XX:12:34:56:78:XX
 *   $ randmac    40:a6:d9:XX:XX:XX   # Apple, Inc.
 *   $ randmac    42:a6:d9:XX:XX:XX   # private
 *
 * Only the MAC address should be passed in; any prefix material will
 * throw off calculations for the -m and -p options. Use sed(1) or
 * something afterwards if additional data massaging is required:
 *
 *   $ randmac -p XX-XX-XX-XX-XX-XX | sed 's/^/01-/'
 * 
 * Lacking a compiler, another option would be something like:
 * 
 *   #!/usr/bin/env perl
 *   my @hex_chars = ( 0 .. 9, 'a' .. 'f' );
 *   my $mac = shift || 'XX:XX:XX:XX:XX:XX';
 *   $mac =~ s/X/$hex_chars[rand @hex_chars]/eg;
 *   print $mac, "\n";
 *
 * Viewing the bits for a given hex value is possible via:
 *
 *   $ perl -e 'printf "%08b\n", 0x40'                
 *   01000000
 *   $ perl -e 'printf "%08b\n", 0x42'
 *   01000010
 *
 * So 42, being private, enables the penultimate bit, while 40, being
 * assigned to Apple, does not. However! There appear to be a few OUI
 * that set the private (locally administered) or broadcast bits:
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
 * conflicting with some actual piece of hardware from one of these
 * vendors would be somewhat low.
 *
 * IPv4 broadcast sets all ones (low odds of randomly rolling); IPv4
 * multicast uses a prefix of 01:00:5e, and IPv6 multicast 33:33. This
 * tool at present takes no effort not to generate something inside
 * these ranges.
 */

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#if defined(linux) || defined(__linux) || defined(__linux__)
#include <sys/types.h>
#include <time.h>
#endif

/* Each 'X' in the input corresponds to 4 bits, these are to pull
 * appropriate amount of bits out of the random data. */
#define MAC_BIT_MASK  15
#define MAC_SEG_SIZE  4

/* How to toggle the appropriate bits for the flags */
#define MAC_MULTICAST 1U<<0
#define MAC_PRIVATE   1U<<1

/* "All the bits" of random(3) are usable, so use as many MAC_SEG_SIZE
 * of the 31 as possible (in hindsight, this is largely a needless and
 * bug infested complication; better to just call random and run with
 * those results). */
#define RAND_MAX_BITS 31

void emit_usage(void);

bool Flag_Multicast;
bool Flag_Private;

int main(int argc, char *argv[])
{
    int ch;
    char mac[] = "XX:XX:XX:XX:XX:XX";
    char *mp;
    unsigned long randval;
    unsigned int position, randbit, rvi;

    while ((ch = getopt(argc, argv, "h?mp")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
            emit_usage();
            /* NOTREACHED */
        case 'm':
            Flag_Multicast = true;
            break;
        case 'p':
            Flag_Private = true;
            break;
        }
    }
    argc -= optind;
    argv += optind;

    if (argv[0] != NULL)
        (void) strncpy(mac, argv[0], sizeof(mac) - 1);

#if defined(linux) || defined(__linux) || defined(__linux__)
    /* something hopefully decent enough */
    srandom(time(NULL) ^ (getpid() + (getpid() << 15)));
#else
    /* assume modern *BSD */
    srandomdev();
#endif

    randval = random();
    rvi = 0;

    mp = mac;
    position = 0;
    while (*mp != '\0') {
        if (*mp == 'X') {
            randbit = (randval >> (rvi++ * MAC_SEG_SIZE)) & MAC_BIT_MASK;

            /* KLUGE will fail if there is prefix material, e.g.
             * 01-XX-XX-XX-XX-XX-XX, but supporting that would
             * necessitate a more complicated parser. */
            if (position == 1) {
                if (Flag_Multicast)
                    randbit |= MAC_MULTICAST;
                else
                    randbit &= ~(MAC_MULTICAST);
                if (Flag_Private)
                    randbit |= MAC_PRIVATE;
                else
                    randbit &= ~(MAC_PRIVATE);
            }
            printf("%x", randbit);

            if (RAND_MAX_BITS - rvi * MAC_SEG_SIZE < MAC_SEG_SIZE) {
                randval = random();
                rvi = 0;
            }
        } else
            putchar(*mp);

        position++;
        mp++;
    }
    putchar('\n');

    exit(EXIT_SUCCESS);
}

void emit_usage(void)
{
    errx(EX_USAGE, "[-m] [-p] 02:03:04:XX:XX:XX");
}
