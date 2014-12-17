          /*   "Tap or untap target artifact, creature, or land."   */

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

long Flag_Offset;               /* -o */
int Flag_Bit;                   /* -b */

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, fd;
    char buf, *epb, *epo;
    ssize_t bytes_read, bytes_written;

    while ((ch = getopt(argc, argv, "b:h?o:")) != -1) {
        switch (ch) {
        case 'b':
            Flag_Bit = strtol(optarg, &epb, 10);
            if (optarg[0] == '\0' || *epb != '\0')
                errx(EX_DATAERR, "could not parse -b bit to twiddle");
            if (Flag_Bit < 0 || Flag_Bit >= CHAR_BIT)
                errx(EX_DATAERR, "option -b out of range");
            break;

        case 'o':
            Flag_Offset = strtol(optarg, &epo, 10);
            if (optarg[0] == '\0' || *epo != '\0')
                errx(EX_DATAERR, "could not parse -o offset value");
            if (Flag_Offset < 0 || Flag_Offset > INT_MAX)
                errx(EX_DATAERR, "option -o out of range");
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

        bytes_read = pread(fd, &buf, (size_t) 1, Flag_Offset);
        if (bytes_read == -1)
            err(EX_IOERR, "pread() failure on '%s'", *argv);
        else if (bytes_read == 0)
            errx(EX_IOERR, "read of '%s': EOF", *argv);
        else if (bytes_read != 1)
            errx(EX_IOERR, "unexpected pread() on '%s'", *argv);

        buf ^= (1 << Flag_Bit);

        bytes_written = pwrite(fd, &buf, (size_t) 1, Flag_Offset);
        if (bytes_written == -1)
            err(EX_IOERR, "pwrite() failure on '%s'", *argv);
        else if (bytes_written != 1)
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
