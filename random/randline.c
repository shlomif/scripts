/* randline - reservoir-sample a random line out of the input */

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);
void pick_line(FILE * fh, ssize_t * counter, char **chosen,
               ssize_t * chosen_length);

#ifdef __OpenBSD__
#define randomly arc4random_uniform
#else
#include <fcntl.h>

uint32_t randomly(uint32_t upper_bound);

int Rand_FD;
#endif

int main(int argc, char *argv[])
{
    char *the_chosen_one = NULL;
    FILE *fh;
    int ch;
    ssize_t counter = 1;
    ssize_t chosen_length, written;
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

#ifndef __OpenBSD__
    if ((Rand_FD = open("/dev/urandom", O_RDONLY)) == -1)
        err(EX_IOERR, "could not open /dev/urandom");
#endif

    if (argc == 0 || strncmp(*argv, "-", (size_t) 2) == 0) {
        pick_line(stdin, &counter, &the_chosen_one, &chosen_length);
    } else {
        while (*argv) {
            if ((fh = fopen(*argv, "r")) == NULL)
                err(EX_IOERR, "could not open '%s'", *argv);
            pick_line(fh, &counter, &the_chosen_one, &chosen_length);
            argv++;
        }
    }

    if (!the_chosen_one) {
        if (counter > 1) {
            errx(1, "no line chosen ??");
        } else {
            exit(1);
        }
    }

    if ((written =
         write(STDOUT_FILENO, the_chosen_one,
               chosen_length)) != chosen_length) {
        if (written == -1) {
            err(EX_IOERR, "error writing chosen line");
        } else {
            errx(EX_IOERR, "incomplete write: expected %ld wrote %ld ??",
                 chosen_length, written);
        }
    }
    /* POSIX compliance (and horrible `while read line` shell bug
     * avoidance): ensure that `(echo a; echo -n c) | randline` always
     * emits an ultimate newline */
    if (the_chosen_one[chosen_length - 1] != '\n') {
        if ((written = write(STDOUT_FILENO, "\n", 1)) != 1) {
            if (written == -1) {
                err(EX_IOERR, "error writing ultimate newline");
            } else {
                errx(EX_IOERR, "incomplete write on newline ??");
            }
        }
    }
    //free(the_chosen_one);
    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: randline [file [file2 ..]|-]\n");
    exit(EX_USAGE);
}

inline void pick_line(FILE * fh, ssize_t * counter, char **chosen,
                      ssize_t * chosen_length)
{
    char *line = NULL;
    char *pick = NULL;
    size_t linesize;
    ssize_t linelen;
    ssize_t line_number = *counter;
    while ((linelen = getline(&line, &linesize, fh)) != -1) {
        /* 1 returns 0 so first line always picked, 2 has 50% of
         * returning 0 so the second line might be picked, and less so
         * so forth on down the line as the line numbers increment.
         * appears no different than using floating point math under
         * various statistical tests */
        if (randomly(line_number) == 0) {
            if (pick != NULL)
                free(pick);
            if ((pick = strndup(line, linesize)) == NULL)
                err(EX_OSERR, "could not copy line");
            *chosen = pick;
            *chosen_length = linelen;
        }
        /* KLUGE unlikely to process this many lines, and it could be
         * said that a 1 in 4294967295+n chance is close enough to 1 in
         * 4294967295 for government work... */
        if (line_number < UINT32_MAX)
            line_number++;
    }
    *counter = line_number;
}

#ifndef __OpenBSD__
uint32_t randomly(uint32_t upper_bound)
{
    uint32_t result;

    if (read(Rand_FD, &result, sizeof(uint32_t)) != sizeof(uint32_t))
        err(EX_IOERR, "read from /dev/urandom failed");

    /* avoid modulo bias; this is probably unnecessary assuming the
     * number of lines processed remains well below UINT32_MAX */
    while (result > UINT32_MAX / upper_bound * upper_bound) {
        if (read(Rand_FD, &result, sizeof(uint32_t)) != sizeof(uint32_t))
            err(EX_IOERR, "read from /dev/urandom failed");
    }

    return result % upper_bound;
}
#endif
