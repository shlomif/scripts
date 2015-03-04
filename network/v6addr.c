/* IPv6 address qualifier (-f, the default) or reverse DNSer (-r).
 * Attempts a manual parse of the v6 address supplied as the first
 * argument, and will report where that parsing goes awry. Otherwise,
 * inet_pton(3) is used for a final opinion on the input and to produce
 * the output address.
 *
 * Briefly tested on OpenBSD/amd64 and Debian/ppc, no apparent endian
 * issues present in this code.
 */

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
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

/* for manual parse; in6_addr instead uses 16 8-bit unsigned ints */
#define QUADS 8
#define S6ADDR_MAX 16

void emit_help(void);
void emit_unparsable(const char *str, const int idx, const char *err_str);

bool Flag_Forward = true;
bool Flag_Reverse = false;
bool Flag_Quiet = false;

enum {
    STATE_START,
    STATE_WANTHEX,
    STATE_WANTCOLON,
};

uint16_t addr[QUADS];

int main(int argc, char *argv[])
{
    char *ap, *ep;
    char tmpstr[5];             /* buffer for parsed segments */
    int ch;
    int curquad = 0;
    int dblcln_idx = -1;        /* where the :: is if any */
    int dblcln_offset = 0;      /* for error output */
    int input_offset = 0;       /* for error output */
    int netmask = 0;
    int state = STATE_START;
    int ret = 0;
    struct in6_addr v6addr;

    while ((ch = getopt(argc, argv, "fhrq")) != -1) {
        switch (ch) {
            /* last of these mentioned wins, in event shell aliases involved */
        case 'f':
            Flag_Forward = true;
            Flag_Reverse = false;
            break;
        case 'r':
            Flag_Reverse = true;
            Flag_Forward = false;
            break;

        case 'q':
            Flag_Quiet = true;
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

    ap = *argv;

    while (1) {
        int advance = 0;

        if (state == STATE_START || state == STATE_WANTHEX) {
            if ((ret = sscanf(ap, "%4[A-Fa-f0-9]%n", tmpstr, &advance)) == 1) {
                /* hex data, convert */
                unsigned long val;
                errno = 0;
                val = strtoul(tmpstr, &ep, 16);
                if (*ep != '\0')
                    emit_unparsable(*argv, input_offset,
                                    "could not parse hex value");
                if ((val == 0 && errno == EINVAL) || val > UINT16_MAX)
                    emit_unparsable(*argv, input_offset, "value out of range");

                addr[curquad++] = (uint16_t) val;

                if (curquad > QUADS) {
                    emit_unparsable(*argv, input_offset, "address is too long");
                }
                ap += advance;
                input_offset += advance;
                state = STATE_WANTCOLON;
                continue;
            }
        }
        if (state == STATE_START || state == STATE_WANTCOLON) {
            if ((ret = sscanf(ap, "%2[:]%n", tmpstr, &advance)) == 1) {
                /* :: mark where zero run happens, increment things */
                if (strnlen(tmpstr, (size_t) 2) == 2) {
                    /* but only once, as cannot be two :: in a v6 addr */
                    if (dblcln_idx == -1) {
                        dblcln_idx = curquad;
                        dblcln_offset = input_offset;

                        ap += advance;
                        input_offset += advance;
                        state = STATE_WANTHEX;
                        continue;
                    }
                } else {
                    /* : just move things along */
                    ap += advance;
                    input_offset += advance;
                    state = STATE_WANTHEX;
                    continue;
                }
            }
        }
        if (ret == EOF)
            break;
        /* % treated like EOF as address might have trailing %lo0 (but
         * inet_pton() does not like that, so NUL it out here). */
        if (*ap == '%') {
            *ap = '\0';
            break;
        }
        /* netmask foo - TODO act on this in output */
        if (*ap == '/') {
            fprintf(stderr, "dbg slash\n");
            if ((ret = sscanf(ap, "/%3d", &netmask)) == 1) {
                fprintf(stderr, "dbg nm %d\n", netmask);
                if (netmask < 0 || netmask > 128)
                    emit_unparsable(*argv, input_offset + 1, "invalid netmask");
            } else {
                emit_unparsable(*argv, input_offset, "invalid netmask");
            }
            *ap = '\0';
            break;
        }
        emit_unparsable(*argv, input_offset, NULL);
    }

    /* :: handling, may need to align certain quads to end */
    if (dblcln_idx > -1) {
        int quads_after = curquad - dblcln_idx;
        int quads_offset = dblcln_idx + quads_after - 1;

        if (quads_after + dblcln_idx > QUADS) {
            emit_unparsable(*argv, dblcln_offset, "invalid double colon");
        } else if (quads_after + dblcln_idx < QUADS) {
            for (int i = 0; i < quads_after; i++) {
                addr[QUADS - 1 - i] = addr[quads_offset - i];
                addr[quads_offset - i] = 0;
            }
        } else {
            /* Nothing for :: to do if no zeros to fill in on either side, as
             * if here have a full eight quads. The :: might otherwise be
             * invalid if there's a longer run of zeros elsewhere, but
             * checking for that would be annoying. */
            if ((addr[dblcln_idx - 1] & 15) != 0 && addr[dblcln_idx] > 0xfff)
                emit_unparsable(*argv, dblcln_offset, "invalid double colon");
        }
    } else if (curquad < QUADS) {
        emit_unparsable(*argv, input_offset, "incomplete address");
    }

    /* A final opinion (but does not show where the address goes awry) */
    if ((ret = inet_pton(AF_INET6, *argv, &v6addr)) != 1) {
        if (ret == -1) {
            emit_unparsable(*argv, input_offset, strerror(errno));
        } else {
            emit_unparsable(*argv, input_offset, "inet_pton() failed");
        }
    }

    /* emit address (using the inet_pton() parsed results) */
    if (Flag_Forward) {
        for (int i = 0; i < S6ADDR_MAX; i++) {
            printf("%02x", v6addr.s6_addr[i]);
            if (i < S6ADDR_MAX - 1 && i % 2 == 1)
                putchar(':');
        }
    } else {
        /* reverse for DNS is slightly trickier */
        for (int i = S6ADDR_MAX - 1; i >= 0; i--) {
            for (int j = 0; j < 2; j++) {
                printf("%1x.", v6addr.s6_addr[i] >> (j * 4) & 15);
            }
        }
        printf("ip6.arpa.");
    }
    putchar('\n');

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: [-f | -r] ipv6-address\n");
    exit(EX_USAGE);
}

void emit_unparsable(const char *str, const int idx, const char *err_str)
{
    if (!Flag_Quiet) {
        if (err_str) {
            warnx("could not parse ipv6-address: %s", err_str);
        } else {
            warnx("could not parse ipv6-address");
        }
        fprintf(stderr, "        %s\n        %*c\n", str, idx + 1, '^');
    }
    exit(EX_DATAERR);
}
