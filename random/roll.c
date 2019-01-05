/* roll - rolls dice */

#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#define ROLL_MIN 2
#define ROLL_MAX 10000
#define ROLL_TIMES_MIN 1
#define ROLL_TIMES_MAX 10000

#ifndef __OpenBSD__
int Rnd_FD;
#endif

void emit_help(void);
int rnd(int range);
unsigned long roll(int number, int sides);
void roll_em(const char *spec);

int main(int argc, char *argv[])
{
    int ch;

    setlocale(LC_ALL, "C");

    while ((ch = getopt(argc, argv, "h?")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 1)
        emit_help();

#ifndef __OpenBSD__
    if ((Rnd_FD = open("/dev/urandom", O_RDONLY)) == -1)
        err(EX_IOERR, "open /dev/urandom failed");
#endif

    while (*argv != NULL) {
        if (*argv[0] == '\0')
            errx(EX_USAGE, "cannot parse empty string");
        if (!isdigit(*argv[0]))
            errx(EX_USAGE, "input must begin with a number");
        roll_em(*argv);
        argv++;
    }

    //close(Rnd_FD);
    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: roll dice-spec [dice-spec ..]\n");
    exit(EX_USAGE);
}

int rnd(int range)
{
    uint32_t result;
#ifdef __OpenBSD__
    result = arc4random_uniform(range);
#else
    uint32_t max = UINT32_MAX / range * range;
    do {
        if (read(Rnd_FD, &result, sizeof(result)) != sizeof(result))
            err(EX_IOERR, "read from /dev/urandom failed");
    } while (result > max);
    result %= range;
#endif
    return (int) result;
}

unsigned long roll(int number, int sides)
{
    unsigned long total = 0;
    while (number--)
        total += rnd(sides) + 1;
    return total;
}

void roll_em(const char *spec)
{
    char *ep;
    int number, sides;
    unsigned long val = 0;
    val = strtoul(spec, &ep, 0);
    if (val < ROLL_TIMES_MIN)
        errx(EX_USAGE, "value below minimum in '%s'", spec);
    if (val > ROLL_TIMES_MAX)
        errx(EX_USAGE, "value above maximum in '%s'", spec);
    if (*ep == '\0') {          /* bare "20" implies 1d20 */
        number = 1;
        if (val < ROLL_MIN)
            errx(EX_USAGE, "sides must be >= %d", ROLL_MIN);
        sides = (int) val;
    } else if (*ep == 'd') {    /* "3d6" form */
        number = (int) val;
        spec = ep;
        spec++;
        val = strtoul(spec, &ep, 0);
        if (spec[0] == '\0' || *ep != '\0')
            errx(EX_USAGE, "stray characters in '%s'", spec);
        if (val < ROLL_MIN)
            errx(EX_USAGE, "value below minimum in '%s'", spec);
        if (val > ROLL_MAX)
            errx(EX_USAGE, "value above maximum in '%s'", spec);
        sides = (int) val;
    } else {
        errx(EX_USAGE, "unknown characters in spec '%s'", spec);
    }
    printf("%lu\n", roll(number, sides));
}
