/*
 * Sometimes segfault. Mostly an educational test case for AFL testing:
 *
 * http://lcamtuf.coredump.cx/afl/
 *
 * Usage might run something along the lines of (optimization tends to compile
 * out the deliberate segfault):
 *

env CC="afl-gcc" CFLAGS="-g -std=c99" make segfaultmaybe
echo 2038-01-02 | ./segfaultmaybe

mkdir inputs findings
echo > inputs/dates <<EOF
1938-09-06
2018-09-05
1977-02-14
1948-05-23
1957-10-26
1926-07-02
1992-11-12
1951-08-15
1986-05-24
EOF
afl-fuzz -i inputs -o findings ./segfaultmaybe

 * Though the specific bug this code contains would be better found by
 * intelligent input, e.g. what happens with years from 0000..9999 or beyond,
 * for example, and especially at boundary conditions around, say, year 2038.
 *
 * Result of analysis: desktop system at work overheated, crashed, and no
 * segfault was found in something like a week of chugging away. Maybe on some
 * server class hardware less shoddily built?
 *
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

const char *Program_Name;
int Return_Value = EXIT_SUCCESS;

const char *File_Name = "-";    /* file name being read from */

void emit_help(void);

int main(int argc, char *argv[])
{
    FILE *fh;
    char *line = NULL;
    char *whoops = "cat";
    int ch;
    size_t linesize = 0;
    ssize_t linelen;
    static struct tm when;

#ifdef __OpenBSD__
    // since OpenBSD 5.4
    Program_Name = getprogname();
#else
    Program_Name = *argv;
#endif

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
        fh = stdin;
    } else {
        if ((fh = fopen(*argv, "r")) == NULL)
            err(EX_IOERR, "could not open '%s'", *argv);
        File_Name = *argv;
    }

    while ((linelen = getline(&line, &linesize, fh)) != -1) {
        /* UTC timezone and anything after %Z would trigger segfaults on some
         * versions of strptime.c in OpenBSD, though that should be fixed in
         * OpenBSD 5.7 and onwards. There's also a strptime bug languishing in
         * OS X land, sigh. */
        if (strptime(line, "%Y-%m-%d", &when) != NULL) {
            /* However, our goal here is much more modest... */
            if (when.tm_year == 138) {
                *whoops = 'b';
            }
            printf("%ld", (long) mktime(&when));
        }
    }
    if (ferror(fh))
        err(EX_IOERR, "error reading '%s'", File_Name);

    exit(Return_Value);
}

void emit_help(void)
{
    const char *shortname;
#ifdef __OpenBSD__
    shortname = Program_Name;
#else
    if ((shortname = strrchr(Program_Name, '/')) != NULL)
        shortname++;
    else
        shortname = Program_Name;
#endif

    fprintf(stderr, "Usage: %s file|-\n", shortname);

    exit(EX_USAGE);
}
