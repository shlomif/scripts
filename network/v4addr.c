/* v4addr - IPv4 address qualifier or reverse DNSer. mostly for
 * compatibility with v6addr so can call this from dynamic dns scripts
 * that need IPv4 reverse addresses */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

/* v4 addresses have four quads a.b.c.d, of at most three digits, and
 * whose digits do not exceed the given max (I'm ignoring the various
 * wacky forms that Linux supports) */
#define QUADS 4
/* NOTE this is set manually in the sscanf, below */
#define QUAD_MAX_DIGITS 3
#define QUAD_MAX_VALUE 255UL

void emit_help(void);
void emit_unparsable(const char *str, const int idx, const char *err_str);

bool Flag_Forward = true;       /* -f, default */
bool Flag_Reverse = false;      /* -r */
bool Flag_Quiet = false;        /* -q */

enum {
    STATE_WANTNUM,
    STATE_WANTDOT,
};

int main(int argc, char *argv[])
{
    char *ap, *ep;
    char tmpstr[QUAD_MAX_DIGITS + 1];   /* buffer for parsed segments */
    int ch;
    int curquad = 0;
    int input_offset = 0;       /* for error output */
    int ret = 0;
    int state = STATE_WANTNUM;
    struct in_addr v4addr;

    while ((ch = getopt(argc, argv, "afhrq")) != -1) {
        switch (ch) {
        case 'a':
            Flag_Forward = true;
            Flag_Reverse = true;
            break;
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

        if (state == STATE_WANTNUM) {
            /* NOTE the magic 3 is QUAD_MAX_DIGITS */
            if ((ret = sscanf(ap, "%3[0-9]%n", tmpstr, &advance)) == 1) {
                unsigned long val;
                errno = 0;
                val = strtoul(tmpstr, &ep, 10);
                if (*ep != '\0')
                    emit_unparsable(*argv, input_offset,
                                    "could not parse digits value");
                if ((val == 0 && errno == EINVAL) || val > QUAD_MAX_VALUE)
                    emit_unparsable(*argv, input_offset, "value out of range");

                if (++curquad == QUADS)
                    break;

                ap += advance;
                input_offset += advance;
                state = STATE_WANTDOT;
                continue;
            } else {
                emit_unparsable(*argv, input_offset, "expected a digit");
            }
        }
        if (state == STATE_WANTDOT) {
            if (*ap == '.') {
                ap++;
                input_offset++;
                state = STATE_WANTNUM;
                continue;
            } else {
                emit_unparsable(*argv, input_offset, "expected a dot");
            }
        }
        /* state value corrupted, or ... */
        emit_unparsable(*argv, input_offset, "unexpected parse state ??");
    }

    if ((ret = inet_pton(AF_INET, *argv, &v4addr)) != 1) {
        if (ret == -1) {
            emit_unparsable(*argv, input_offset, strerror(errno));
        } else {
            emit_unparsable(*argv, input_offset, "inet_pton() failed");
        }
    }

    /* little endian arch? big endian? other? normalize */
    v4addr.s_addr = htonl(v4addr.s_addr);

    if (Flag_Forward) {
        for (int i = 0; i < 4; i++) {
            printf("%u", v4addr.s_addr >> ((3 - i) * 8) & 0xff);
            if (i < 3)
                putchar('.');
        }
        putchar('\n');
    }
    if (Flag_Reverse) {
        for (int i = 0; i < 4; i++) {
            printf("%u.", v4addr.s_addr >> (i * 8) & 0xff);
        }
        printf("in-addr.arpa.\n");
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: [-a | -f | -r] ipv4-address\n");
    exit(EX_USAGE);
}

void emit_unparsable(const char *str, const int idx, const char *err_str)
{
    if (!Flag_Quiet) {
        if (err_str) {
            warnx("could not parse ipv4-address: %s", err_str);
        } else {
            warnx("could not parse ipv4-address");
        }
        fprintf(stderr, "        %s\n        %*c\n", str, idx + 1, '^');
    }
    exit(EX_DATAERR);
}
