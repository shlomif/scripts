/* Randomly flips (close to) a given percentage of the bits the given file(s) */

#ifdef __linux__
#define _XOPEN_SOURCE 500
typedef unsigned char u_char;
#include <bsd/stdlib.h>
#endif

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#define BUFSIZE 8192

// equation "percent / 100 = u / UINT32_MAX" solved for u
//   perl -e 'for (1..100) { printf "%d, ", $_*4294967295/100 }'
uint32_t percent2uint32[100] = {
    42949672, 85899345, 128849018, 171798691, 214748364, 257698037, 300647710,
    343597383, 386547056, 429496729, 472446402, 515396075, 558345748,
    601295421, 644245094, 687194767, 730144440, 773094113, 816043786,
    858993459, 901943131, 944892804, 987842477, 1030792150, 1073741823,
    1116691496, 1159641169, 1202590842, 1245540515, 1288490188, 1331439861,
    1374389534, 1417339207, 1460288880, 1503238553, 1546188226, 1589137899,
    1632087572, 1675037245, 1717986918, 1760936590, 1803886263, 1846835936,
    1889785609, 1932735282, 1975684955, 2018634628, 2061584301, 2104533974,
    2147483647, 2190433320, 2233382993, 2276332666, 2319282339, 2362232012,
    2405181685, 2448131358, 2491081031, 2534030704, 2576980377, 2619930049,
    2662879722, 2705829395, 2748779068, 2791728741, 2834678414, 2877628087,
    2920577760, 2963527433, 3006477106, 3049426779, 3092376452, 3135326125,
    3178275798, 3221225471, 3264175144, 3307124817, 3350074490, 3393024163,
    3435973836, 3478923508, 3521873181, 3564822854, 3607772527, 3650722200,
    3693671873, 3736621546, 3779571219, 3822520892, 3865470565, 3908420238,
    3951369911, 3994319584, 4037269257, 4080218930, 4123168603, 4166118276,
    4209067949, 4252017622, 4294967295
};

// -i, -n flag names taken from cmp(1)
off_t Flag_Skip;                // -i SKIP
off_t Flag_Limit;               // -n LIMIT
uint32_t Flag_Odds;             // -o odds

char *buf;

void corrupt_file(const char *filename);
void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;

    while ((ch = getopt(argc, argv, "h?i:n:o:")) != -1) {
        switch (ch) {
        case 'i':
            Flag_Skip =
                (off_t) flagtoul(ch, optarg, 1UL, (unsigned long) LONG_MAX);
            break;
        case 'n':
            Flag_Limit =
                (off_t) flagtoul(ch, optarg, 1UL, (unsigned long) LONG_MAX);
            break;
        case 'o':
            Flag_Odds = percent2uint32[flagtoul(ch, optarg, 1UL, 100UL) - 1];
            break;
        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc == 0 || Flag_Odds == 0)
        emit_help();

    if ((buf = malloc(BUFSIZE)) == NULL)
        err(EX_OSERR, "could not malloc buffer");

    while (*argv) {
        corrupt_file(*argv);
/* PORTABILITY FreeBSD 11, Mac OS X 10.11, libbsd 0.6.0 on Centos7 all
 * have stir, while OpenBSD does not (that happens behind the scenes) */
#ifndef __OpenBSD__
        arc4random_stir();
#endif
        argv++;
    }

    exit(EXIT_SUCCESS);
}

// not fully standalone: relies on the globals buf, Flag_*
void corrupt_file(const char *filename)
{
    int fd;
    off_t filepos, filesize;

    int buffer_modified = 0;
    ssize_t readsize, wrotesize, steps;
    uint32_t *bp, randtail;

    if ((fd = open(filename, O_RDWR)) == -1)
        err(EX_IOERR, "could not open '%s'", filename);
    if ((filesize = lseek(fd, 0, SEEK_END)) == -1)
        err(EX_IOERR, "could not lseek");
    // pread/pwrite used so the lseek to the end is irrelevant
    filepos = 0;

    if (Flag_Skip != 0) {
        if (Flag_Skip >= filesize)
            errx(1, "will not skip entire file '%s'", filename);
        filepos = Flag_Skip;
    }
    if (Flag_Limit != 0 && Flag_Limit < filesize
        && filepos + Flag_Limit < filesize) {
        filesize = filepos + Flag_Limit;
    }

    while (filepos < filesize) {
        readsize = filesize - filepos;
        if (readsize > BUFSIZE)
            readsize = BUFSIZE;
        if ((readsize = pread(fd, buf, readsize, filepos)) == -1)
            err(EX_IOERR, "pread failed on '%s'", filename);
        if (readsize == 0)      // EOF
            break;

        // corrupt buffer using 32-bit chunks
        bp = (uint32_t *) buf;
        steps = readsize / sizeof(uint32_t);
        while (steps--) {
            if (arc4random() < Flag_Odds) {
                *bp ^= arc4random();
                buffer_modified = 1;
            }
            bp++;
        }
        // ... though not everything will line up on 32-bit boundaries ...
        if ((steps = readsize % sizeof(uint32_t)) != 0) {
            if (arc4random() < Flag_Odds) {
                randtail = arc4random();
                while (steps--) {
                    buf[readsize - 1 - steps] = (randtail >> (steps * 8)) & 255;
                }
                buffer_modified = 1;
            }
        }

        if (buffer_modified) {
            if ((wrotesize = pwrite(fd, buf, readsize, filepos)) == -1)
                err(EX_IOERR, "pwrite failed on '%s'", filename);
            if (wrotesize != readsize)
                errx(EX_IOERR, "incomplete pwrite on '%s'", filename);
            buffer_modified = 0;
        }

        filepos += readsize;
    }
    close(fd);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: corrupt [-i skip] -o odds [-n limit] file ..\n");
    exit(EX_USAGE);
}
