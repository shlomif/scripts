/* Reservoir-sample a random line out of the input */

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);
void pick_line(FILE * fh, ssize_t * counter, char **chosen,
               ssize_t * chosen_length);

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

    if (!the_chosen_one)
        errx(1, "no line chosen??");
    if ((written =
         write(STDOUT_FILENO, the_chosen_one,
               chosen_length)) != chosen_length) {
        if (written == -1) {
            err(EX_IOERR, "error writing chosen line");
        } else {
            errx(1, "incomplete write: expected %ld wrote %ld??", chosen_length,
                 written);
        }
    }
    /* POSIX compliance (and horrible `while read line` shell bug
     * avoidance): ensure that `(echo a; echo -n c) | randline` always
     * emits an ultimate newline. */
    if (the_chosen_one[chosen_length - 1] != '\n')
        putchar('\n');
    free(the_chosen_one);
    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: randline [file [file2 ..]|-]\n");
    exit(EX_USAGE);
}

void pick_line(FILE * fh, ssize_t * counter, char **chosen,
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
         * Appears no different than using floating point math under
         * various statistical tests. */
        if (arc4random_uniform(line_number) == 0) {
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
