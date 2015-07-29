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

/*
 # deltas are done by byte, so will be +/-0..255; this guard is for the deltas
 # list in the event that number of items does not properly match the number of
 # files provided (which is unknown until after option processing). Another
 # method would be to save the delta string, and parse that once the number of
 # files is known...
 */
#define DELTA_EOL INT_MIN

#define MAX_DELTAS 1024

void emit_help(void);
int *parse_deltas(const char *s);

int main(int argc, char *argv[])
{
    int ch, *deltas, *fds;
    char **fbufs;
    off_t fsize;
    struct stat statbuf;

    deltas = NULL;
    while ((ch = getopt(argc, argv, "h?d:")) != -1) {
        switch (ch) {

        case 'd':
            deltas = parse_deltas(optarg);
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

    if (argc <= 0 || !deltas)
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
        if (deltas[i] == DELTA_EOL)
            errx(EX_DATAERR, "too few deltas for number of input files");
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

int *parse_deltas(const char *s)
{
    char *ep, *sp;
    int cur_d, *deltas, max_d;
    long long value;

    if (!s || *s == '\0') {
        warnx("could not parse deltas: empty option argument");
        emit_help();
    }

    max_d = 8;
    if ((deltas = calloc((size_t) max_d, sizeof(int))) == NULL)
        err(EX_OSERR, "could not calloc() %d deltas", max_d);

    cur_d = 0;
    sp = (char *) s;
    while (1) {
        errno = 0;
        value = strtoll(sp, &ep, 10);
        if (s[0] == '\0')
            errx(EX_DATAERR, "could not parse delta value");
        if (errno == ERANGE && (value == LLONG_MIN || value == LLONG_MAX))
            errx(EX_DATAERR, "delta value out of range");
        if (value < -255 || value > 255)
            errx(EX_DATAERR, "delta value beyond bounds");

        deltas[cur_d++] = (int) value;

        while (cur_d >= max_d) {
            max_d <<= 1;
            if ((deltas =
                 realloc(deltas, (size_t) max_d * sizeof(int))) == NULL)
                err(EX_OSERR, "could not realloc() %d deltas", max_d);
            if (max_d > MAX_DELTAS)
                errx(1, "will not allocate more than %d deltas", MAX_DELTAS);
        }

        if (*ep == '\0')
            break;
        else
            sp = ep;
    }

    deltas[cur_d] = DELTA_EOL;
    return deltas;
}
