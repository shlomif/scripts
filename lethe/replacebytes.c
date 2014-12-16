/************************************************************************
 * Quick (but dangerous?) method of overwriting arbitrary bytes in the
 * files specified with arbitrary data from a source file. The files are
 * modified in-place, which may not be the most sensible thing to do.
 * Atomicity would involve some combination of mktemp(3) and rename(2).
 * But that, in turn, might have complications should some fancy ACL or
 * security context only be associated with the now previous inode.
 *
 * Usage:
 *   replacebytes -S 1024 -f srcfile fixfile1 fixfile2 ..
 *   replacebytes -I 123 -O 456 -S 789 -f src fix
 *   replacebytes -s -f src2 fix2
 *   replacebytes -m 'the text' -O 1234 fix3
 *
 * Which would replace the first 1024 bytes of `fixfile*' with the first
 * 1024 bytes of `srcfile', or take 789 bytes from `src' at offset 123
 * and write it at offset 456 of `fix', or write the entire contents of
 * `src2' over the beginning of `fix2', or instead use the -m option to
 * specify the replacement text. See also dd(1), hexdump(1), the
 * File::ReplaceBytes Perl module, and your backups. Backups are nice.
 * You have backups, right?
 */

#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);

char *Flag_Srcfile;             /* -f */
unsigned long Flag_Offset;      /* -O */
unsigned long Flag_Trunc;       /* -t */

int main(int argc, char *argv[])
{
    int ch, fd;
    char *bufp = NULL;
    char *epo, *ept;
    int buf_len = 0;
    off_t seek_len;
    int write_len;

    while ((ch = getopt(argc, argv, "h?f:m:O:t:")) != -1) {
        switch (ch) {
        case 'f':
            if ((Flag_Srcfile = strndup(optarg, (size_t) PATH_MAX)) == NULL)
                err(EX_SOFTWARE, "could not copy the -f flag");
            break;

        case 'm':
            if ((buf_len = asprintf(&bufp, "%s", optarg)) == -1)
                err(EX_SOFTWARE, "could not copy the -m flag");
            break;

        case 'O':
            errno = 0;
            Flag_Offset = strtoul(optarg, &epo, 10);
            if (optarg[0] == '\0' || *epo != '\0')
                errx(EX_DATAERR, "could not parse -O offset option");
            if (errno == ERANGE && Flag_Offset == ULONG_MAX)
                errx(EX_DATAERR, "option -O out of range");
            break;

        case 't':
            errno = 0;
            Flag_Trunc = strtoul(optarg, &ept, 10);
            if (optarg[0] == '\0' || *ept != '\0')
                errx(EX_DATAERR, "could not parse -t truncate option");
            if (errno == ERANGE && Flag_Trunc == ULONG_MAX)
                errx(EX_DATAERR, "option -t out of range");
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
    if (argc == 0) {
        warnx("need files to operate on");
        emit_help();
    }

    /********************************************************************
     *
     * Read to buffer from the source (-f) file.
     *
     */

    if (!bufp) {
        if ((fd = open(Flag_Srcfile, O_RDONLY)) == -1)
            err(EX_IOERR, "could not open() -f '%s'", Flag_Srcfile);

        if ((seek_len = lseek(fd, (off_t) 0, SEEK_END)) == -1)
            err(EX_IOERR, "could not lseek() to end -f '%s'", Flag_Srcfile);
        if (seek_len < 1)
            errx(EX_DATAERR, "no data in -f '%s'", Flag_Srcfile);

        if (seek_len >= INT_MAX)
            errx(EX_DATAERR, "too much data in -f '%s'", Flag_Srcfile);

        if ((bufp = malloc((size_t) seek_len)) == NULL)
            err(EX_OSERR, "could not malloc() buffer for -f '%s' contents",
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

    /********************************************************************
     *
     * Write buffer to specified location in any files mentioned.
     *
     */

    while (*argv) {
        if ((fd = open(*argv, O_WRONLY)) == -1)
            err(EX_IOERR, "could not open() output file '%s'", *argv);

        /* PORTABILITY "some platforms allow for nbytes to range between
         * SSIZE_MAX and SIZE_MAX -2" per OpenBSD write(2). Worry about
         * this if actually see it in the wild. */
        if ((write_len =
             (int) pwrite(fd, bufp, (size_t) buf_len,
                          (off_t) Flag_Offset)) != buf_len) {
            if (write_len > -1)
                errx(EX_IOERR,
                     "could not write %d to output file '%s': got instead %d",
                     buf_len, *argv, write_len);
            else
                err(EX_IOERR, "error writing to output file '%s'", *argv);
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
            "Usage: replacebytes [-f file|-m str] [-I offset] file [file2 ...]\n");
    exit(EX_USAGE);
}
