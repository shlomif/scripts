/*
# CHIsq test integers belonging to a given contiguous range. C not R
# because a) a second opinion is good and b) I'm too lazy to figure out
# how to make R honor sparse input, where a particular bucket might have
# no values. Otherwise, use the "chisq.test" in R or the "equichisq"
# arlet of my r-fu script.
#
# The following should fail, as there are six buckets (implicit 0 for
# minimum through 5, inclusive) and the perl only produces numbers for
# five of those buckets (0..4):
#
#   perl -E 'for (1..1000) { say int rand 5}' | chii -M 5
#
# (It could also fail for reasons unrelated to the missing bucket, or it
# could pass if there are not enough trials to reveal the lack.)
#
# Now, if you're actually testing the fitness of non-cryptographic hash
# functions, consider instead the "smhasher" project.
*/

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

// GNU Science Library
#include <gsl/gsl_cdf.h>

double Flag_Fitness = 0.05;     // -F @ 95% confidence by default
long long Flag_Min;             // -m
long long Flag_Max;             // -M

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;

    char *ep;
    long long count = 0;
    long long delta, *intstore, value;
    double expected, pvalue, sq;

    char *line = NULL;
    char *lp;
    FILE *fh;
    size_t linesize = 0;
    ssize_t linelen;
    ssize_t lineno = 1;

    while ((ch = getopt(argc, argv, "F:h?m:M:")) != -1) {
        switch (ch) {

        case 'F':
            Flag_Fitness = flagtod(ch, optarg, 0.0, 1.0);
            break;

        case 'm':
            Flag_Min = flagtoll(ch, optarg, LLONG_MIN, LLONG_MAX);
            break;

        case 'M':
            Flag_Max = flagtoll(ch, optarg, LLONG_MIN, LLONG_MAX);
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

    if ((Flag_Min == 0 && Flag_Max == 0) || Flag_Min == Flag_Max
        || Flag_Min > Flag_Max)
        emit_help();

    if (argc == 0 || strncmp(*argv, "-", (size_t) 2) == 0) {
        fh = stdin;
    } else {
        if ((fh = fopen(*argv, "r")) == NULL)
            err(EX_IOERR, "could not open '%s'", *argv);
    }

    // inclusive, need buckets for all possible numbers
    delta = Flag_Max - Flag_Min + 1;

    if ((intstore = calloc((size_t) delta, sizeof(long long))) == NULL)
        err(EX_OSERR, "could not calloc() for %lld long longs", delta);

    while ((linelen = getline(&line, &linesize, fh)) != -1) {
        // so can parse multiple numbers per line, assuming they are
        // isspace(3) delimited (downside: complicates the parsing)
        lp = line;
        while (*lp != '\0') {
            errno = 0;
            value = strtoll(lp, &ep, 10);
            if (*lp == '\0')
                errx(EX_DATAERR, "could not parse long long at line %ld",
                     lineno);
            if (errno == ERANGE && (value == LLONG_MIN || value == LLONG_MAX))
                errx(EX_DATAERR, "value out of range at line %ld", lineno);
            if (value < Flag_Min || value > Flag_Max)
                errx(EX_DATAERR, "value beyond bounds at line %ld", lineno);

            intstore[value - Flag_Min]++;
            count++;

            // nom up not-strtoll() material so parser does not get wedged
            // on trailing material such as `echo -n 0 1 2 3 4" "` 
            while (*ep != '+' && *ep != '-' && !isdigit(*ep)) {
                if (*ep == '\0' || *ep == '\n') {
                    *ep = '\0';
                    break;
                }
                ep++;
            }
            lp = ep;
        }
        lineno++;
    }
    if (count == 0)
        errx(EX_DATAERR, "could not parse any numbers");
    if (ferror(fh))
        err(EX_IOERR, "ferror() reading input");

    // presume even distribution of values
    expected = count / (double) delta;

    sq = 0.0;
    for (long long v = 0; v < delta; v++) {
        sq += pow(intstore[v] - expected, 2.0) / expected;
    }
    pvalue = gsl_cdf_chisq_Q(sq, (double) delta - 1);
    printf("p-value %.6g%s\n", pvalue, pvalue <= Flag_Fitness ? " *" : "");

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: chii [-F fit] -m min -M max [file|-]\n");
    exit(EX_USAGE);
}
