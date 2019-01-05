/* repcharcount - counts repeats of the same character */

#include <sys/types.h>

#include <err.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define BUFSIZE 8192

void emit_help(void);
void report(unsigned long count, int c);

int main(int argc, char *argv[])
{
    char buf[BUFSIZE], *fname;
    int ch, fd, previous;
    ssize_t amount;
    unsigned long count = 1;

    setlocale(LC_ALL, "C");

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

    if (argc == 0 || (argc == 1 && strncmp(*argv, "-", (size_t) 2) == 0)) {
        fd = STDIN_FILENO;
        fname = "-";
    } else {
        if ((fd = open(*argv, O_RDONLY)) == -1)
            err(EX_IOERR, "could not open '%s'", *argv);
        fname = *argv;
    }

    if ((amount = read(fd, buf, 1)) != 1)
        err(EX_IOERR, "read() on %s failed", fname);
    previous = buf[0];

    while (1) {
        amount = read(fd, buf, BUFSIZE);
        if (amount > 0) {
            for (ssize_t i = 0; i < amount; i++) {
                if (buf[i] != previous) {
                    report(count, previous);
                    previous = buf[i];
                    count = 0;
                }
                count++;
            }
        } else if (amount == 0) {       // EOF
            break;
        } else if (amount == -1) {
            err(EX_IOERR, "read() on %s failed", fname);
        } else {
            errx(EX_IOERR, "unexpected read() on %s: %ld", fname, amount);
        }
    }
    if (count > 0)
        report(count, previous);

    exit(EXIT_SUCCESS);
}

inline void emit_help(void)
{
    fprintf(stderr, "Usage: repcharcount [file|-]\n");
    exit(EX_USAGE);
}

inline void report(unsigned long count, int c)
{
    if (isprint(c) && !isspace(c)) {
        printf("%lu %c\n", count, c);
    } else {
        printf("%lu 0x%02X\n", count, c);
    }
}
