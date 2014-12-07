/*
 * Parses timestamps in the input data via strptime(3), and emits that data in
 * epoch form (or with format as formatted by strftime(3)) to standard out.
 */

/* ugh, linux */
#ifdef __linux__
#define _GNU_SOURCE
#define _XOPEN_SOURCE
#endif

#include <sys/types.h>

#include <err.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

/* Limit on realloc() of strftime output buffer, mostly for if someone
 * specifies a stupidly long strftime() format. */
#define OUTBUF_LEN_MIN 32
#define OUTBUF_LEN_MAX 8192

#define INPUT_FORMAT_MAXLEN 99

/* Command Line Options */
char *Flag_Input_Format;        /* -f for strptime - required */
bool Flag_Global;               /* -g multiple times per line */
char *Flag_Output_Format;       /* -o for strftime */
bool Flag_Suppress;             /* -s suppress other output */
bool Flag_Custom_Year;          /* -y or -Y YYYY custom year */

/* for -[yY] supplied data and pre-filler for subsequent strptime results */
struct tm Default_Tm;

const char *File_Name = "-";    /* file name being read from */

void emit_help(void);
void parseline(char *line, const ssize_t linenum);

int main(int argc, char *argv[])
{
    FILE *fh;
    char *line = NULL;
    int ch, ret;
    size_t linesize = 0;
    ssize_t linelen;
    ssize_t linenum = 1;
    time_t now;

    if (!setlocale(LC_ALL, ""))
        errx(EX_USAGE, "setlocale(3) failed: check the locale settings");

    while ((ch = getopt(argc, argv, "f:gh?o:syY:")) != -1) {
        switch (ch) {
        case 'f':
            if ((ret = asprintf(&Flag_Input_Format, "%s", optarg))
                == -1)
                err(EX_SOFTWARE, "asprintf(3) could not copy -f flag");
            if (ret > INPUT_FORMAT_MAXLEN)
                errx(EX_DATAERR, "-f flag longer than %d", INPUT_FORMAT_MAXLEN);
            break;

        case 'g':
            Flag_Global = true;
            break;

        case 'o':
            if ((ret = asprintf(&Flag_Output_Format, "%s", optarg)) == -1)
                err(EX_SOFTWARE, "asprintf(3) could not copy -o flag");
            if (ret > INPUT_FORMAT_MAXLEN)
                errx(EX_DATAERR, "-o flag longer than %d", INPUT_FORMAT_MAXLEN);
            break;

        case 's':
            Flag_Suppress = true;
            break;

        case 'y':
            if (time(&now) == (time_t) - 1)
                errx(EX_OSERR, "time(3) could not obtain current time??");
            if (localtime_r(&now, &Default_Tm) == NULL)
                errx(EX_OSERR, "localtime_r(3) failed??");
            Flag_Custom_Year = true;
            break;

        case 'Y':
            if (!strptime(optarg, "%Y", &Default_Tm))
                errx(EX_USAGE, "strptime(3) could not parse year from -Y flag");
            Flag_Custom_Year = true;
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

    if (!Flag_Input_Format)
        emit_help();

    /* Due to crazy behavior on Mac OS X (see also guard for it, below), and
     * otherwise there are less expensive syscalls that can better deal with
     * epoch values. */
    if (strncmp(Flag_Input_Format, "%s", (size_t) 2) == 0)
        errx(EX_DATAERR, "%%s is not supported as input format");

    if (argc == 0 || strncmp(*argv, "-", (size_t) 2) == 0) {
        fh = stdin;
    } else {
        if ((fh = fopen(*argv, "r")) == NULL)
            err(EX_IOERR, "could not open '%s'", *argv);
        File_Name = *argv;
    }

    while ((linelen = getline(&line, &linesize, fh)) != -1) {
        parseline(line, linenum);
        linenum++;
    }
    if (ferror(fh))
        err(EX_IOERR, "error reading file");

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr,
            "Usage: epochal [-sg] [-y|-Y yyyy] -f input-format [-o output-fmt] [file|-]\n"
            "  Pass data in via standard input. See strftime(3) for formats.\n");
    exit(EX_USAGE);
}

void parseline(char *line, const ssize_t linenum)
{
    char *past_date_p;
    size_t ret;
    static char *outbuf;
    static size_t outbuf_len = OUTBUF_LEN_MIN;
    static struct tm when;

    if (!outbuf)
        if ((outbuf = malloc(outbuf_len)) == NULL)
            err(EX_OSERR, "malloc() failed to create output buffer");

    /* Pre-fill with zeros or what -[yY] or other flags set; this may cause
     * problems if strptime resets fields expected to be set to what a flag
     * set, but otherwise need to set something to avoid gibberish values.
     * However, since current flags are for things the input data lacks, will
     * let strptime results take priority over the global defaults. */
    when = Default_Tm;

    while (*line != '\0') {
        if (*line == '\n') {
            putchar('\n');
            break;
        }

        if ((past_date_p = strptime(line, Flag_Input_Format, &when)) != NULL) {
            /* Uhh so yeah about that %s thing on Mac OS X. Guard for it.
             * This was reported in Apple Bug 15753871 maaaaaany months ago but
             * hey I guess there's more important things to deal with? */
            if (past_date_p - line < 1)
                errx(EX_SOFTWARE, "error: zero-width timestamp parse");

            if (!Flag_Output_Format) {
                printf("%ld", (long) mktime(&when));
            } else {
                while ((ret =
                        strftime(outbuf, outbuf_len, Flag_Output_Format, &when))
                       == 0) {
                    outbuf_len <<= 1;
                    if (outbuf_len > OUTBUF_LEN_MAX)
                        errx(EX_SOFTWARE,
                             "strftime() output too large for buffer %d at %s:%ld",
                             OUTBUF_LEN_MAX, File_Name, linenum);
                    if ((outbuf = realloc(outbuf, outbuf_len)) == NULL)
                        err(EX_OSERR,
                            "realloc() could not resize output buffer to %ld",
                            outbuf_len);
                }
                fwrite(outbuf, ret, (size_t) 1, stdout);
            }

            if (!Flag_Global) {
                if (Flag_Suppress) {
                    putchar('\n');
                } else {
                    printf("%s", past_date_p);
                }

                /* no global search means we're done with the line */
                break;

            } else {
                /* spacer necessary between just the strftimes */
                if (Flag_Suppress)
                    putchar(' ');
            }

            when = Default_Tm;
            line = past_date_p;
        } else {
            /* charwise until strptime finds something, or not */
            if (!Flag_Suppress) {
                putchar(*line);
            }
            line++;
        }
    }
}
