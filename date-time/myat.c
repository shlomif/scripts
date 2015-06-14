/*
 * Due to at(1) accepting a variety of curious date formats none of
 * which I otherwise use and reading the manual page for at(1) every
 * time I need to use it isn't fun. (Figuring out the strftime(3) magic
 * to avoid the 'garbled time' error from at(1) was annoying enough, but
 * less annoying than using some JavaScript app on Google.)
 */

#include <err.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

/* Oh, the silly olden days with their middle-endian Month Day Year--no
 * wonder I cannot memorize this! */
#define AT_TIMEFORMAT "%H:%M %b %d %Y"

/* for strftime result; minimum is sized to AT_TIMEFORMAT in my locale */
#define BUF_LEN_MIN 18
#define BUF_LEN_MAX 72

bool Flag_Quiet;                /* -q */

void emit_help(void);

/* free zeroing of struct members (though creates mday gotcha (that is not
 * relevant here as %d must be supplied by caller, and if they want to set that
 * to 0, well, more power to them)) */
struct tm when;

int main(int argc, char *argv[])
{
    int ch;
    size_t time_str_len = BUF_LEN_MIN;
    char *time_str;

    while ((ch = getopt(argc, argv, "h?q")) != -1) {
        switch (ch) {
        case 'q':
            Flag_Quiet = true;
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

    if (argc < 1 || argc > 2)
        emit_help();

    if (!Flag_Quiet) {
        /* but note that this limit is easy to get past in a shell via
         *   while `mkdir aaaaaaaa`; do cd aaaaaaaa; done
         * but in that case this here code gives up as is right and proper.
         */
        char cwd[PATH_MAX], *wdp, *token;
        if (!getcwd(cwd, (size_t) PATH_MAX))
            err(EX_IOERR, "getcwd() failed");
        wdp = strdup(cwd);
        while ((token = strsep(&wdp, "/")) != NULL) {
            if (strncmp(token, "tmp", (size_t) 4) == 0) {
                fprintf(stderr, "warning: CWD contains 'tmp': %s\n", cwd);
                break;
            }
        }
    }

    if ((time_str = malloc(time_str_len)) == NULL)
        err(EX_OSERR, "malloc() failed to create output buffer");

    if (strptime(*argv, "%Y-%m-%d", &when)) {
        if (argc == 2)
            if (!strptime(*++argv, "%H:%M", &when))
                errx(EX_USAGE, "could not parse HH:MM");
    } else if (strptime(*argv, "%H:%M", &when)) {
        time_t epoch;
        struct tm *now;
        if (time(&epoch) == (time_t) - 1)
            err(EX_OSERR, "time() failed");
        if ((now = localtime(&epoch)) == NULL)
            err(EX_OSERR, "localtime() failed");
        when.tm_year = now->tm_year;
        when.tm_mon = now->tm_mon;
        when.tm_mday = now->tm_mday;
    } else {
        /* in contrast to the "garbled time" error from at(1) ... */
        errx(EX_USAGE, "need YYYY-MM-DD or HH:MM as first argument");
    }

    while (strftime(time_str, time_str_len, AT_TIMEFORMAT, &when) < 1) {
        time_str_len <<= 1;
        if (time_str_len > BUF_LEN_MAX)
            errx(EX_SOFTWARE, "strftime() output too large for buffer %d",
                 BUF_LEN_MAX);
        if ((time_str = realloc(time_str, time_str_len)) == NULL)
            err(EX_OSERR,
                "realloc() could not resize output buffer to %ld",
                time_str_len);
    }

    if (execlp("at", "at", time_str, (char *) 0) == -1)
        err(EX_OSERR, "could not exec at");

    exit(EX_OSERR);             /* NOTREACHED due to exec or so we hope */
}

void emit_help(void)
{
    fprintf(stderr, "Usage: myat [-q] YYYY-MM-DD [HH:MM]\n");
    fprintf(stderr, "       myat [-q] HH:MM      (assumes today)\n");
    exit(EX_USAGE);
}
