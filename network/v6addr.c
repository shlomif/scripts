/* IPv6 address qualifier (-f, the default) or reverse DNSer (-r).
 * Mostly just C practice, find a proper library or use sipcalc or
 * something else instead, as this code is short on tests.
 *
 * Requires -std=c99 to compile.
 */

/* ugh, linux */
#if defined(linux) || defined(__linux) || defined(__linux__)
extern char *optarg;
extern int optind, opterr, optopt;
#define _GNU_SOURCE
#endif

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

#define QUADS 8

void emit_help(void);
void emit_unparsable(const char *str, const int idx, const char *err_str);

bool Flag_Forward = true;
bool Flag_Reverse = false;

enum {
    STATE_START,
    STATE_WANTHEX,
    STATE_WANTCOLON,
};

uint16_t addr[QUADS];

int
main(int argc, char *argv[])
{
    int ch;

    while ((ch = getopt(argc, argv, "fr")) != -1) {
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
	default:
	    emit_help();
	    /* NOTREACHED */
	}
    }
    argc -= optind;
    argv += optind;

    if (argc == 0)
	emit_help();

    char *ap = *argv;
    char tmpstr[5];		/* buffer for parsed segments */
    int curquad = 0;
    int dblcln_idx = -1;	/* where the :: is if any */
    int dblcln_offset = 0;	/* for error output */
    int input_offset = 0;	/* for error output */
    int state = STATE_START;
    int ret = 0;

    while (1) {
	int advance = 0;

	if (state == STATE_START || state == STATE_WANTHEX) {
	    if ((ret = sscanf(ap, "%4[A-Fa-f0-9]%n", tmpstr, &advance)) == 1) {
		/* hex data, convert */
		unsigned long val;
		errno = 0;
		val = strtoul(tmpstr, NULL, 16);
		if ((val == 0 && errno == EINVAL) || val > UINT16_MAX) {
		    /* should not happen? EINVAL maybe UNPORTABLE */
		    emit_unparsable(*argv, input_offset, "value out of range");
		} else {
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
	/* % treated like EOF as address might have trailing %lo0 */
	if (ret == EOF || *ap == '%')
	    break;

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
    if ((ret = inet_pton(AF_INET6, *argv, NULL)) != 1) {
	if (ret == -1) {
	    emit_unparsable(*argv, input_offset, strerror(errno));
	} else {
	    emit_unparsable(*argv, input_offset, "inet_pton() failed");
	}
    }

    /* emit address */
    if (Flag_Forward) {
	for (int i = 0; i < QUADS; i++) {
	    printf("%04x", addr[i]);
	    if (i < QUADS - 1)
		putchar(':');
	}
    } else {
	/* reverse for DNS is slightly trickier */
	for (int i = QUADS - 1; i >= 0; i--) {
	    if (addr[i] == 0) {
		printf("0.0.0.0.");
	    } else if (addr[i] == UINT16_MAX) {
		printf("f.f.f.f.");
	    } else {
		for (int j = 0; j < 4; j++) {
		    printf("%1x.", addr[i] >> (j * 4) & 15);
		}
	    }
	}
	printf("ip6.arpa.");
    }
    putchar('\n');

    exit(EXIT_SUCCESS);
}

void
emit_help(void)
{
    fprintf(stderr, "Usage: [-f | -r] ipv6-address\n");
    exit(EX_USAGE);
}

void
emit_unparsable(const char *str, const int idx, const char *err_str)
{
    if (err_str) {
	warnx("error: could not parse ipv6-address: %s", err_str);
    } else {
	warnx("error: could not parse ipv6-address");
    }
    fprintf(stderr, "        %s\n        %*c\n", str, idx + 1, '^');
    exit(EX_DATAERR);
}
