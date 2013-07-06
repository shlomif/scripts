/*
 * Quick (but dangerous?) method of overwriting the first N bytes of files
 * with data from a source file. The files are modified in-place, which may
 * not be the most sensible thing to do. An alternative would be to write to
 * a mktemp(3) output file, then rename(2) that to the original file if both
 * the header and remaining data of the original file to fix can be copied to
 * the temp file without error. But that, in turn, might have complications
 * should some fancy ACL or security context only be associated with the now
 * previous inode.
 *
 * Usage:
 *   replacehdr -S 1024 -f srcfile fixfile1 fixfile2 ..
 *   replacehdr -O 4096 -S 1024 -f src fix       # non-zero offsets of edits!
 *
 * See also dd(1), hexdump(1), and your backups. Backups are nice. You have
 * backups, right?
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

void emit_help(void);

char Flag_Srcfile[PATH_MAX + 1];        /* -f */
unsigned int Flag_Offset;       /* -O */
unsigned int Flag_Size;         /* -S */

int main(int argc, char *argv[])
{
    char *bufp;
    int ch, fd;
    ssize_t hdr_size;

    while ((ch = getopt(argc, argv, "h?f:O:S:")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
            emit_help();
            /* NOTREACHED */
        case 'f':
            /* PORTABILITY olden Unix will lack these. Meh. */
            if (strnlen(optarg, PATH_MAX) > PATH_MAX)
                errx(EX_DATAERR, "-f file exceeds PATH_MAX (%d)", PATH_MAX);
            strncpy(Flag_Srcfile, optarg, PATH_MAX);
            Flag_Srcfile[sizeof(Flag_Srcfile) - 1] = '\0';
            break;
        case 'O':
            if (sscanf(optarg, "%d", &Flag_Offset) != 1)
                errx(EX_DATAERR, "could not parse -O offset option");
            break;
        case 'S':
            if (sscanf(optarg, "%d", &Flag_Size) != 1)
                errx(EX_DATAERR, "could not parse -S size option");
            break;
        default:
            emit_help();
        }
    }
    argc -= optind;
    argv += optind;

    if (argc == 0 || Flag_Size == 0 || Flag_Srcfile[0] == '\0')
        emit_help();

    if ((bufp = malloc(Flag_Size)) == NULL)
        err(EX_OSERR, "could not malloc() buffer");

    if ((fd = open(Flag_Srcfile, O_RDONLY)) == -1)
        err(EX_IOERR, "could not open() -f file");
    /*
     * PORTABILITY same concerns as write(2) below, but since a fixed header
     * size is mandatory, do want to fail if cannot read exactly that size.
     */
    if ((hdr_size = read(fd, bufp, Flag_Size)) != Flag_Size) {
        if (hdr_size > -1)
            errx(EX_IOERR, "could not read %d from -f file: got %ld",
                 Flag_Size, hdr_size);
        else
            err(EX_IOERR, "error reading header from -f file");
    }
    while (argc-- > 0) {
        if (strnlen(*argv, PATH_MAX) > PATH_MAX)
            errx(EX_DATAERR, "file %s exceeds PATH_MAX (%d)",
                 *argv, PATH_MAX);
        if ((fd = open(*argv, O_WRONLY)) == -1)
            err(EX_IOERR, "could not open() %s", *argv);
        if (Flag_Offset > 0)
            if (lseek(fd, Flag_Offset, SEEK_SET) == -1)
                err(EX_IOERR, "could not lseek() to %d of %s",
                    Flag_Offset, *argv);
        /*
         * PORTABILITY "some platforms allow for nbytes to range between
         * SSIZE_MAX and SIZE_MAX -2" per OpenBSD write(2). Worry about
         * this if actually see it in the wild.
         */
        if ((hdr_size = write(fd, bufp, Flag_Size)) != Flag_Size) {
            if (hdr_size > -1)
                errx(EX_IOERR, "could not write %d to %s: got %ld",
                     Flag_Size, *argv, hdr_size);
            else
                err(EX_IOERR, "error writing to %s", *argv);
        }
        /* but must seek to end, else (unsurprising in hindsight) truncation */
        if (lseek(fd, 0, SEEK_END) == -1)
            err(EX_IOERR, "could not lseek() to end of %s", *argv);
        if (close(fd) == -1)
            err(EX_IOERR, "error closing %s", *argv);

        argv++;
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr,
            "Usage: replacehdr -f file [-O offset] -S size file [file2 ..]\n");
    exit(EX_USAGE);
}
