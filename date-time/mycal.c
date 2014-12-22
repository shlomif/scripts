/*
 * Mostly motivated by BSD cal(1) lacking the -3 option. Also, to better learn
 * the ctime(3) routines.
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

const char *Program_Name;

void emit_help(void);
void whatmonth(struct tm *date);

int main(int argc, char *argv[])
{
    int ch;
    time_t epoch;
    struct tm *when;

    Program_Name = *argv;

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

    if (time(&epoch) == (time_t) - 1)
        err(EX_OSERR, "time() failed");
    if ((when = localtime(&epoch)) == NULL)
        err(EX_OSERR, "localtime() failed");

    /* Ragged ends of months may cause wrap-around problems if run on 31st and
     * whoops next month doesn't have that so bump to some other month, haha!
     * Basically, datetime code without massive unit tests checking for these
     * sorts of edge cases should not be trusted (or it should be of no great
     * surprise when things do break).
     */
    when->tm_mday = 1;

    when->tm_mon -= 1;
    whatmonth(when);
    when->tm_mon += 1;
    whatmonth(when);
    when->tm_mon += 1;
    whatmonth(when);

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    const char *shortname;
    if ((shortname = strrchr(Program_Name, '/')) != NULL)
        shortname++;
    else
        shortname = Program_Name;

    fprintf(stderr, "Usage: %s\n", shortname);

    exit(EX_USAGE);
}

void whatmonth(struct tm *date)
{
    char monthnum[3];
    char yearnum[5];            // FIXME Y10K bug
    pid_t pid;
    time_t t;

    /* this dance to actually resolve the month increments */
    t = mktime(date);
    date = localtime(&t);

    pid = fork();
    if (pid == -1) {
        err(EX_OSERR, "could not fork()");

    } else if (pid == 0) {      // child
        if (strftime((char *) &monthnum, (size_t) 3, "%m", date) < 1)
            errx(EX_OSERR, "could not strftime() month");
        if (strftime((char *) &yearnum, (size_t) 5, "%Y", date) < 1)
            errx(EX_OSERR, "could not strftime() year");

        if (execlp("cal", "cal", &monthnum, &yearnum, (char *) 0) == -1)
            err(EX_OSERR, "could not execlp() cal");

    } else {                    // parent
        if (wait(NULL) == -1)
            err(EX_OSERR, "wait() error");
    }
}
