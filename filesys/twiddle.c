/* twiddle - twiddles the specified bit at the specfied offset in the
 * specified file(s)
 * 
 *   "Tap or untap target artifact, creature, or land."
 *     -- Magic: The Gathering                           */

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

int Flag_Bit;                   /* -b */
long Flag_Offset;               /* -o */

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, fd;
    char buf;
    ssize_t amnt_read, amnt_written;

    while ((ch = getopt(argc, argv, "b:h?o:")) != -1) {
        switch (ch) {
        case 'b':
            Flag_Bit = (int) flagtoul(ch, optarg, 0UL, 7UL);
            break;
        case 'o':
            // KLUGE no "off_t max" so guess at something suitable
            Flag_Offset =
                (long) flagtoul(ch, optarg, 0UL, (unsigned long) LONG_MAX);
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

    if (argc == 0)
        emit_help();

    while (*argv) {
        if ((fd = open(*argv, O_RDWR)) == -1)
            err(EX_IOERR, "could not open '%s'", *argv);

        amnt_read = pread(fd, &buf, (size_t) 1, Flag_Offset);
        if (amnt_read == -1)
            err(EX_IOERR, "pread() failure on '%s'", *argv);
        else if (amnt_read == 0)
            errx(EX_IOERR, "read of '%s': EOF", *argv);
        else if (amnt_read != 1)
            errx(EX_IOERR, "unexpected pread() on '%s'", *argv);

        buf ^= (1 << Flag_Bit);

        amnt_written = pwrite(fd, &buf, (size_t) 1, Flag_Offset);
        if (amnt_written == -1)
            err(EX_IOERR, "pwrite() failure on '%s'", *argv);
        else if (amnt_written != 1)
            errx(EX_IOERR, "unequal pwrite() on '%s'", *argv);

        if (close(fd) == -1)
            err(EX_IOERR, "problem closing '%s'", *argv);

        argv++;
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: twiddle -b bit -o offset file [file1 ..]\n");
    exit(EX_USAGE);
}
