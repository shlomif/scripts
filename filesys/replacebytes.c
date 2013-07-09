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
 *
 * Which would replace the first 1024 bytes of `fixfile*' with the first
 * 1024 bytes of `srcfile', or take 789 bytes from `src' at offset 123
 * and write it at offset 456 of `fix', or write the entire contents of
 * `src2' over the beginning of `fix2'. See also dd(1), hexdump(1), and
 * your backups. Backups are nice. You have backups, right?
 */

#include <sys/types.h>

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);

bool Flag_Auto_Size;            /* -s */
char Flag_Srcfile[PATH_MAX + 1];        /* -f */
unsigned long Flag_Input_Offset;        /* -I */
unsigned long Flag_Offset;      /* -O */
unsigned long Flag_Size;        /* -S */

int main(int argc, char *argv[])
{
    int ch, fd;

    off_t offset = 0;           /* for -s related lseek auto-sizing */

    char *bufp;
    ssize_t hdr_size;

    while ((ch = getopt(argc, argv, "h?f:I:O:S:s")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
            emit_help();
            /* NOTREACHED */
        case 'f':
            /* PORTABILITY olden Unix will lack. Meh. Otherwise, do not
             * care if truncate, as I don't even know what they're doing
             * with a path that long. Something naughty, I suppose. */
            strncpy(Flag_Srcfile, optarg, PATH_MAX);
            Flag_Srcfile[sizeof(Flag_Srcfile) - 1] = '\0';
            break;
        case 'I':
            if (sscanf(optarg, "%li", &Flag_Input_Offset) != 1)
                errx(EX_DATAERR, "could not parse -I offset option");
            break;
        case 'O':
            if (sscanf(optarg, "%li", &Flag_Offset) != 1)
                errx(EX_DATAERR, "could not parse -O offset option");
            break;
        case 'S':
            if (sscanf(optarg, "%li", &Flag_Size) != 1)
                errx(EX_DATAERR, "could not parse -S size option");
            break;
        case 's':
            Flag_Auto_Size = true;
            break;
        default:
            emit_help();
        }
    }
    argc -= optind;
    argv += optind;

    if (argc == 0 || (Flag_Size == 0 && Flag_Auto_Size == false)
        || Flag_Srcfile[0] == '\0')
        emit_help();
    if (Flag_Size > 0 && Flag_Auto_Size == true) {
        warnx("cannot use both -s and -S");
        emit_help();
    }

    /********************************************************************
     *
     * Read to buffer from the source (-f) file.
     *
     */

    if ((fd = open(Flag_Srcfile, O_RDONLY)) == -1)
        err(EX_IOERR, "could not open() source %s", Flag_Srcfile);

    if (Flag_Auto_Size) {
        if ((offset = lseek(fd, 0, SEEK_END)) == -1)
            err(EX_IOERR, "could not lseek() to end of %s", Flag_Srcfile);
        if (offset < 1)
            errx(EX_DATAERR, "no data in source %s", Flag_Srcfile);
        if (Flag_Input_Offset > 0) {
            offset -= Flag_Input_Offset;
            if (offset < 1)
                errx(EX_DATAERR, "no data beyond offset in source %s",
                     Flag_Srcfile);
        }
        if (offset > 0)
            Flag_Size = offset;
    }

    if ((bufp = malloc(Flag_Size)) == NULL)
        err(EX_OSERR, "could not malloc() buffer");

    /* PORTABILITY same concerns as write(2) below, but since a fixed
     * header size is mandatory, do want to fail if cannot read
     * exactly that. */
    if ((hdr_size =
         pread(fd, bufp, Flag_Size, Flag_Input_Offset)) != Flag_Size) {
        if (hdr_size > -1) {
            errx(EX_IOERR, "could not read %ld from %s: got instead %ld",
                 Flag_Size, Flag_Srcfile, hdr_size);
        } else {
            err(EX_IOERR, "error reading header from %s", Flag_Srcfile);
        }
    }
    close(fd);

    /********************************************************************
     *
     * Write buffer to specified location in any files mentioned.
     *
     */

    while (*argv) {
        if ((fd = open(*argv, O_WRONLY)) == -1)
            err(EX_IOERR, "could not open() %s", *argv);

        /* PORTABILITY "some platforms allow for nbytes to range between
         * SSIZE_MAX and SIZE_MAX -2" per OpenBSD write(2). Worry about
         * this if actually see it in the wild. */
        if ((hdr_size =
             pwrite(fd, bufp, Flag_Size, Flag_Offset)) != Flag_Size) {
            if (hdr_size > -1)
                errx(EX_IOERR, "could not write %ld to %s: got instead %ld",
                     Flag_Size, *argv, hdr_size);
            else
                err(EX_IOERR, "error writing to %s", *argv);
        }

        if (close(fd) == -1)
            err(EX_IOERR, "error closing %s", *argv);

        argv++;
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr,
            "Usage: replacebytes -f file [-I offset] [-S size | -s] [-O offset] file [..]\n");
    exit(EX_USAGE);
}
