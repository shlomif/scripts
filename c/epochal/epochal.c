/*
 * Parse timestamps via strptime(3) in data passed via standard input,
 * and emits the time data formatted via strftime(3) to standard out.
 */

#ifdef __linux__
#define _XOPEN_SOURCE
#endif

#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define LINE_MAX 8192

enum returns { R_OKAY, R_USAGE, R_SYS_ERR };
int exit_status = R_OKAY;

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
    int ch, c;
    char input_buf[LINE_MAX];
    unsigned int ib_len;
    unsigned long linenum;

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
            if ((int) time(&now) == -1)
                errx(R_SYS_ERR, "time(3) could not obtain current time");
            localtime_r(&now, &filler);
            break;
        case 'Y':
            Yflag = optarg;
            if (strptime(Yflag, "%Y", &filler) == NULL)
                errx(R_USAGE,
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

    ib_len = 0;
    linenum = 1;
    while ((c = getchar()) != EOF) {
        input_buf[ib_len++] = c;
        if (ib_len > LINE_MAX)
            errx(R_SYS_ERR, "line too long (>%d) at line %ld",
                 LINE_MAX, linenum);

        if (c == '\n') {
            input_buf[ib_len] = '\0';
            parseline(input_buf, ib_len, linenum);

            ib_len = 0;
            linenum++;
        }
    }
    if (ib_len > 0)
        parseline(input_buf, ib_len, linenum);

    exit(exit_status);
}

void parseline(char *input_buf, unsigned int ib_len, unsigned long linenum)
{
    char *ibp, *past_date_p;
    struct tm when = { };
    unsigned int j;
    char out_buf[LINE_MAX];
    unsigned long sret;

    ibp = input_buf;

    for (j = 0; j < ib_len; j++) {
        past_date_p = strptime(ibp, fflag, &when);

        if (past_date_p != NULL) {
            if (when.tm_year == 0 && (yflag || Yflag != NULL)) {
                when.tm_year = filler.tm_year;
            }
            when.tm_isdst = -1; /* Try to auto-handle DST */

            sret = (long) strftime(out_buf, LINE_MAX, oflag, &when);
            if (sret > 0) {
                printf("%s%s", out_buf,
                       (gflag && sflag) ? " "
                       : gflag ? "" : sflag ? "\n" : past_date_p);
            } else {
                exit_status = R_SYS_ERR;
                warnx("unexpected return from strftime(3) at line %ld",
                      linenum);
            }
            if (gflag) {
                j = (int) (past_date_p - input_buf - 1);
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
    if (gflag)
        putchar('\n');

    return;
}

void usage(void)
{
    fprintf(stderr,
            "Usage: epochal [-sg] [-y|-Y yyyy] -f input-format [-o output-fmt]\n"
            "  Pass data in via standard input. See strftime(3) for formats.\n");
    exit(R_USAGE);
}
