/* macnorm - normalizes MAC addresses */

#include <ctype.h>
#include <err.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#define NOTHING 0

enum { A_NUMBER, A_SEPARATOR };

int Flag_Octets = 6;            /* -O */
char *Flag_Prefix;              /* -p */
int Flag_Sep = ':';             /* -s */
int Flag_TrailingGarbage;       /* -T */

void emit_help(void);
void normalize_mac(const char *input, int octets, char *out, int ofsep);

int main(int argc, char *argv[])
{
    char *buf;
    int ch;

    while ((ch = getopt(argc, argv, "h?O:p:qs:T")) != -1) {
        switch (ch) {
        case 'O':
            Flag_Octets = (int) flagtoul(ch, optarg, 1UL, 100UL);
            break;
        case 'p':
            Flag_Prefix = optarg;
            break;
        case 's':
            Flag_Sep = *optarg;
            break;
        case 'T':
            Flag_TrailingGarbage = 1;
            break;
        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    if (argc < 1)
        emit_help();
    argv += optind;

    if ((buf = malloc(Flag_Octets * 3 * sizeof(char) + 1)) == NULL)
        err(EX_OSERR, "malloc failed");

    while (*argv) {
        normalize_mac(*argv, Flag_Octets, buf, Flag_Sep);
        if (Flag_Prefix)
            printf("%s", Flag_Prefix);
        printf("%s", buf);
        argv++;
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: macnorm [-p prefix] [-s sep] macaddr [..]\n");
    exit(EX_USAGE);
}

void normalize_mac(const char *input, int octets, char *out, int ofsep)
{
    char *ip;
    int expect = A_NUMBER;
    int offset;
    int onum = 1;
    int sep = -1;
    unsigned int value;

    if (*input == '\0')
        errx(EX_DATAERR, "cannot parse empty string");
    /* disallow leading garbage. also prevents negative hex numbers from
     * being supplied to sscanf */
    if (!isxdigit(*input))
        errx(EX_DATAERR, "string must begin with xdigit");

    ip = (char *) input;

    while (1) {
        if (expect == A_NUMBER) {
            if (sscanf(ip, "%2X%n", &value, &offset) != 1)
                errx(EX_DATAERR, "did not match an xdigit");
            ip += offset;
            sprintf(out, "%02x", value);
            out += 2;
            expect = A_SEPARATOR;
        } else {
            if (onum >= octets)
                break;
            if (sep > 0) {
                if (*ip == sep)
                    ip++;
                else
                    errx(EX_DATAERR, "expected separator not found");
            } else if (sep == NOTHING) {
                ;
            } else {
                /* allow for most any separator (including nothing)
                 * provided that it is used consistently */
                if (*ip == '\0') {
                    errx(EX_DATAERR, "NUL may not be used as separator");
                } else if (isxdigit(*ip)) {
                    sep = NOTHING;
                } else {
                    sep = *ip;
                    ip++;
                }
            }
            if (ofsep > 0) {
                *out = ofsep;
                out++;
            }
            expect = A_NUMBER;
            onum++;
        }
    }
    if (!Flag_TrailingGarbage && *ip != '\0')
        errx(EX_DATAERR, "trailing garbage");
    *out++ = '\n';
    *out = '\0';
}
