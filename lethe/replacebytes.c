/************************************************************************
 * Quick (but dangerous!) method of overwriting arbitrary bytes in the
 * files specified with arbitrary data. The files are modified in-place,
 * which may not be the most sensible thing to do. Atomicity would
 * involve some combination of mktemp(3) and rename(2). But that, in
 * turn, might have complications should some fancy ACL or security
 * context only be associated with the now previous inode.
 *
 * Probably more sensibly done with dd(1).
 */

#include <sys/types.h>

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

void emit_help(void);

char *Flag_Srcfile;             // -f
unsigned long Flag_Offset;      // -O
unsigned long Flag_Trunc;       // -t

char *bufp;                     // -m or from -f srcfile
int buf_len;

int main(int argc, char *argv[])
{
    int ch, fd;
    off_t seek_len;
    int write_len;

    while ((ch = getopt(argc, argv, "f:m:O:t:h?")) != -1) {
        switch (ch) {
        case 'f':
            Flag_Srcfile = optarg;
            break;
        case 'm':
            bufp = optarg;
            buf_len = strnlen(optarg, (size_t) ARG_MAX);
            break;
        case 'O':
            Flag_Offset = flagtoul(ch, optarg, 0UL, ULONG_MAX);
            break;
        case 't':
            Flag_Trunc = flagtoul(ch, optarg, 0UL, ULONG_MAX);
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

    if (bufp && Flag_Srcfile) {
        warnx("cannot mix -m and -f flags");
        emit_help();
    }
    if (!bufp && !Flag_Srcfile) {
        warnx("need one of -m or -f flags");
        emit_help();
    }
    if (argc == 0) {
        warnx("need a file (or files) to operate on");
        emit_help();
    }

    /* Read to buffer from the source (-f) file. */
    if (!bufp) {
        if ((fd = open(Flag_Srcfile, O_RDONLY)) == -1)
            err(EX_IOERR, "could not open() -f '%s'", Flag_Srcfile);

        if ((seek_len = lseek(fd, (off_t) 0, SEEK_END)) == -1)
            err(EX_IOERR, "could not lseek() to end of '%s'", Flag_Srcfile);
        if (seek_len < 1)
            errx(EX_DATAERR, "no data in '%s'", Flag_Srcfile);

        if (seek_len >= INT_MAX)
            errx(EX_DATAERR, "too much data in -f '%s'", Flag_Srcfile);

        if ((bufp = malloc((size_t) seek_len)) == NULL)
            err(EX_OSERR, "could not malloc() buffer for contents of '%s'",
                Flag_Srcfile);

        /* PORTABILITY same concerns as write(2) below, but since a fixed
         * header size is mandatory, do want to fail if cannot read
         * exactly that. */
        if ((buf_len =
             (int) pread(fd, bufp, (size_t) seek_len, (off_t) 0)) != seek_len) {
            if (buf_len > -1) {
                errx(EX_IOERR,
                     "could not read %lld from -f '%s': got instead %d",
                     seek_len, Flag_Srcfile, buf_len);
            } else {
                err(EX_IOERR, "error reading header from -f '%s'",
                    Flag_Srcfile);
            }
        }
        close(fd);
    }

    /* Write buffer to specified location in any files mentioned. */
    while (*argv) {
        if ((fd = open(*argv, O_WRONLY)) == -1)
            err(EX_IOERR, "could not open() output file '%s'", *argv);

        /* PORTABILITY "some platforms allow for nbytes to range between
         * SSIZE_MAX and SIZE_MAX -2" per OpenBSD write(2). Worry about
         * this if actually see it in the wild. */
        if (buf_len > 0) {
            if ((write_len =
                 (int) pwrite(fd, bufp, (size_t) buf_len,
                              (off_t) Flag_Offset)) != buf_len) {
                if (write_len > -1)
                    errx(EX_IOERR,
                         "did not write %d to output file '%s': instead %d ??",
                         buf_len, *argv, write_len);
                else
                    err(EX_IOERR, "error writing to output file '%s'", *argv);
            }
        }

        if (Flag_Trunc > 0)
            if (ftruncate(fd, (off_t) Flag_Trunc) != 0)
                err(EX_IOERR, "could not truncate() outpuat file '%s'", *argv);

        if (close(fd) == -1)
            err(EX_IOERR, "error closing output file '%s'", *argv);

        argv++;
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr,
            "Usage: replacebytes [-f file|-m str] [-O offset] [-t] file [file2 ...]\n");
    exit(EX_USAGE);
}
