/* chii - CHIsq test integers belonging to a given contiguous range. C
 * not R because a) a second opinion is good and b) I'm too lazy to
 * figure out how to make R honor sparse input, where a particular
 * bucket might have no values. otherwise, use the "chisq.test" in R or
 * the "equichisq" arlet of my r-fu script
 *
 * the following should fail, as there are six buckets (implicit 0 for
 * minimum through 5, inclusive) and the Perl only produces numbers for
 * five of those buckets (0..4)
 *
 *   perl -E 'for (1..1000) { say int rand 5}' | chii -M 5
 *
 * (it could also fail for reasons unrelated to the missing bucket, or
 * it could pass if there are not enough trials to reveal the lack)
 *
 * if you are actually testing the fitness of non-cryptographic hash
 * functions, consider instead the "smhasher" project */

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <locale.h>
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

double chisq(long long valuec, long long *values, long long minv,
             long long maxv, long long sum);
void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;

    char *ep;
    long long number_of_nums = 0;
    long long range, *numstore, value;

    char *line = NULL;
    char *lp;
    FILE *fh;
    size_t linesize = 0;
    ssize_t linelen;
    ssize_t lineno = 1;

    double pvalue;

#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    setlocale(LC_ALL, "C");

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
    range = Flag_Max - Flag_Min + 1;

    if ((numstore = calloc((size_t) range, sizeof(long long))) == NULL)
        err(EX_OSERR, "could not calloc() for %lld long longs", range);

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

            numstore[value - Flag_Min]++;
            number_of_nums++;

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
    if (number_of_nums == 0)
        errx(EX_DATAERR, "could not parse any numbers");
    if (ferror(fh))
        err(EX_IOERR, "ferror() reading input");

    pvalue = chisq(range, numstore, Flag_Min, Flag_Max, number_of_nums);
    printf("p-value %.6g%s\n", pvalue, pvalue <= Flag_Fitness ? " FAIL" : "");

    exit(EXIT_SUCCESS);
}

double chisq(long long valuec, long long *values, long long minv,
             long long maxv, long long sum)
{
    unsigned long delta, v;
    double expected, sum_of_squares;

    delta = maxv - minv + 1;
    expected = sum / (double) delta;

    sum_of_squares = 0.0;
    for (v = 0; v < valuec; v++) {
        sum_of_squares += pow(values[v] - expected, 2.0) / expected;
    }
    return gsl_cdf_chisq_Q(sum_of_squares, (double) delta - 1.0);
}

void emit_help(void)
{
    fputs("Usage: chii [-F fit] -m min -M max [file|-]\n", stderr);
    exit(EX_USAGE);
}
