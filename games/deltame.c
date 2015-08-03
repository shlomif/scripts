/*
# Searches for the listed numeric deltas in identical locations of a list of
# input files. A hypothetical use-case would be to look for changes in memory
# dumps of, say, an Apple //e emulator, where the lives of a character went
# from 2 to 3 to 2 to 1 to 0:
#
#   deltame -d '1 -1 -1 -1' cs2 c23 c32 c21 c10 | mostfreq
#
# where `mostfreq` is something like `sort | uniq -c | sort -nr | head`
# and thus:
# 
#   4 034B
#   2 8BBD
#   2 8AD2
#   2 88FF
#   2 87F8
#   2 87F2
#   2 87EF
#   2 87EC
#   2 87D4
#   2 87CB
#
# Memory address 034B is therefore of particular interest.
#
# Note that values that are not exclusive to a particular byte are more
# difficult to spot; these require care in the selection of the delta--
# going from, say, 1492 to 492 gold has these byte-wise deltas:
#
#   % perl -E 'say((1492 & 0xFF)-(492 & 0xFF))'    
#   -24
#   % perl -E 'say((1492 >> 8 & 0xFF)-(492 >> 8 & 0xFF))'
#   4
#
# Watchpoints to confirm suspicions will also be of use.
#
# A less involved test looks something like:
#
#   $ xxd a; xxd b
#   0000000: 6361 7439 646f 670a                      cat9dog.
#   0000000: 6361 7438 646f 670a                      cat8dog.
#   $ ./deltame -d '-1 1' a b a 
#   0003
#   0003
*/

#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#define MAX_DELTAS 1024UL

void emit_help(void);
int *parse_deltas(const char *s);

int main(int argc, char *argv[])
{
    char **fbufs;
    int ch, *fds;
    long long *deltas;
    off_t fsize;
    size_t deltacount;
    struct stat statbuf;

    deltacount = 0;
    deltas = NULL;

    while ((ch = getopt(argc, argv, "h?d:")) != -1) {
        switch (ch) {

        case 'd':
            flagtololls(ch, optarg, -255LL, 255LL, &deltas, &deltacount, 1UL, MAX_DELTAS);
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

    if (argc <= 0 || deltacount == 0)
        emit_help();

    if ((fds = calloc((size_t) argc, sizeof(int))) == NULL)
        err(EX_OSERR, "could not calloc() list of file descriptors");
    if ((fbufs = calloc((size_t) argc, sizeof(char *))) == NULL)
        err(EX_OSERR, "could not calloc() list of file buffer pointers");

    /* all files assumed of equal size as first */
    if (stat(*argv, &statbuf) == -1)
        err(EX_IOERR, "could not stat '%s'", *argv);
    fsize = statbuf.st_size;
    if (fsize <= 0)
        errx(EX_DATAERR, "file '%s' is empty", *argv);
    if (fsize > INT_MAX)
        errx(EX_DATAERR, "file '%s' is too large", *argv);

    if (argc > (int) deltacount + 1) {
        warnx("notice: more files than deltas");
        argc = (int) deltacount + 1;
    }

    /* TODO could use less memory by only having two images in memory at
     * once, or to improve the logic so deltas are done between all of
     * the images or that the logic is applied across all the files, and
     * not just for the two files and location in question. */
    for (int i = 0; i < argc; i++) {
        if ((fds[i] = open(argv[i], O_RDONLY)) == -1)
            err(EX_IOERR, "could not open '%s'", argv[i]);
        if ((fbufs[i] = malloc((size_t) fsize * sizeof(char))) == NULL)
            err(EX_OSERR, "could not malloc() file buffer %d", i);
        if (read(fds[i], fbufs[i], (size_t) fsize) != fsize)
            err(EX_IOERR, "could not read exactly %lld from '%s'", fsize,
                argv[i]);
    }

    for (int i = 0; i < argc - 1; i++) {
        for (off_t w = 0; w < fsize; w++) {
            if ((fbufs[i + 1][w] - fbufs[i][w]) == (int) deltas[i]) {
                printf("%04X\n", (unsigned int) w);
            }
        }
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: deltame -d 'd1 ...' file1 file2 ...\n");
    exit(EX_USAGE);
}
