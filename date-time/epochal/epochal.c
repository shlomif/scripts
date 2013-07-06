/*
 * Parse timestamps via strptime(3) in data passed via standard input,
 * and emits the time data formatted via strftime(3) to standard out.
 */

#ifdef __linux__
#define _XOPEN_SOURCE
#endif

#include <sys/types.h>

#include <err.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#define LINE_MAX 8192

int exit_status = EXIT_SUCCESS;

/* Command Line Options */
char *fflag;                    /* Input format (for strptime) */
int gflag;                      /* Do time check multiple times per line? */
char *oflag;                    /* Output format (for strftime) */
int sflag;                      /* Suppress output (just timestamps out)? */
int yflag;                      /* Auto-fill in year (from current year)? */
char *Yflag;                    /* Specify year */

time_t now;                     /* for when -y set */
struct tm filler;               /* for -y or -Y supplied data */

void parseline(char *, unsigned int, unsigned long);
void usage(void);

int main(int argc, char *argv[])
{
    int ch;
    char input_buf[LINE_MAX];
    unsigned long linenum;

    if (!setlocale(LC_ALL, ""))
        errx(EX_USAGE, "setlocale(3) failed: check the locale settings");

    while ((ch = getopt(argc, argv, "f:go:syY:")) != -1) {
        switch (ch) {
        case 'f':
            fflag = optarg;
            break;
        case 'g':
            gflag = 1;
            break;
        case 'o':
            oflag = optarg;
            break;
        case 's':
            sflag = 1;
            break;
        case 'y':
            yflag = 1;
            if (time(&now) == -1)
                errx(EX_OSERR, "time(3) could not obtain current time");
            localtime_r(&now, &filler);
            break;
        case 'Y':
            Yflag = optarg;
            if (!strptime(Yflag, "%Y", &filler))
                errx(EX_USAGE,
                     "strptime(3) could not parse year from -Y argument");
            break;
        default:
            ;
        }
    }
    argc -= optind;
    argv += optind;

    if (fflag == NULL || (yflag && Yflag != NULL))
        usage();
    if (oflag == NULL)
        oflag = "%s";           /* Default to epoch for output */

    linenum = 1;
    while (1) {
        if (fgets(input_buf, LINE_MAX, stdin) == NULL) {
            if (!feof(stdin))
                err(EX_IOERR, "could not read line %ld", linenum);
            break;
        } else {
            parseline(input_buf, strlen(input_buf), linenum);
            linenum++;
        }
    }

    exit(exit_status);
}

void parseline(char *input_buf, unsigned int ib_len, unsigned long linenum)
{
    char *ibp, *past_date_p;
    struct tm when = { };
    unsigned int j;
    char out_buf[LINE_MAX];
    size_t sret;

    ibp = input_buf;

    for (j = 0; j < ib_len; j++) {
        if (input_buf[j] == '\n') {
            putchar('\n');
            break;
        }

        past_date_p = strptime(ibp, fflag, &when);

        if (past_date_p) {
            if (when.tm_year == 0 && (yflag || Yflag != NULL)) {
                when.tm_year = filler.tm_year;
            }
            when.tm_isdst = -1; /* Try to auto-handle DST */

            sret = strftime(out_buf, LINE_MAX, oflag, &when);
            if (sret > 0) {
                printf("%s%s", out_buf,
                       (gflag && sflag) ? " "
                       : gflag ? "" : sflag ? "\n" : past_date_p);
            } else {
                exit_status = EX_OSERR;
                warnx("unexpected return from strftime(3) at line %ld",
                      linenum);
            }
            if (gflag) {
                j = past_date_p - input_buf - 1;
                ibp = past_date_p;
            } else {
                break;
            }
        } else {
            ibp++;
            if (!sflag)
                putchar(input_buf[j]);
        }
    }

    return;
}

void usage(void)
{
    fprintf(stderr,
            "Usage: epochal [-sg] [-y|-Y yyyy] -f input-format [-o output-fmt]\n"
            "  Pass data in via standard input. See strftime(3) for formats.\n");
    exit(EX_USAGE);
}
