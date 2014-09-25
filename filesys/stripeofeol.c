/*
 * Snips any ultimate linefeed chars (\r and \n) from the named files.
 *
 * This is silly programming practice; files on Unix often need that ultimate
 * newline, as otherwise shell `while` loops and such loose that last line.
 * Hence certain editors warning if the file lacks a trailing newline, or
 * certain cron(8) implementations not running the last line of the file, etc.
 *
 *   % (echo hi; echo -n there) | while read line; do echo $line; done
 *   hi
 *   % 
 *
 */

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#define BUF_SIZE 512

void trim_file(const int fd, const char *file);
void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, fd;

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

    if (argc < 1)
        emit_help();

    while (*argv) {
        if ((fd = open(*argv, O_RDWR)) == -1)
            err(EX_IOERR, "could not open '%s'", *argv);
        trim_file(fd, *argv);
        if (close(fd) == -1)
            err(EX_IOERR, "could not close '%s'", *argv);
        argv++;
    }

    exit(EXIT_SUCCESS);
}

/* Read chunks of the file from end to the beginning, truncate at first non-
 * newline character or 0 if beginning of file reached. */
void trim_file(const int fd, const char *file)
{
    char *bp, buf[BUF_SIZE];
    long byte_count, i, offset, read_size, read_where, total_size, trunc_size;

    if ((total_size = lseek(fd, (off_t) 0, SEEK_END)) < 0)
        err(EX_IOERR, "could not seek '%s'", file);
    if (total_size == 0)
        return;                 /* empty file, nothing to do */

    offset = trunc_size = total_size;

    while (offset > 0) {
        read_where = offset - BUF_SIZE;
        read_size = BUF_SIZE;
        if (read_where < 0) {
            /* not a full buffer remaining at head of file, adjust... */
            read_size = BUF_SIZE + read_where;
            read_where = 0;
        }

        byte_count = pread(fd, &buf, (size_t) read_size, read_where);

        if (byte_count == 0)
            errx(EX_IOERR, "unexpected EOF on pread of '%s'", file);
        else if (byte_count == -1)
            err(EX_IOERR, "pread failed on '%s'", file);
        else if (byte_count < -1)
            errx(EX_IOERR, "unexpected return from pread of '%s': %ld", file,
                 byte_count);
        else {
            bp = buf + byte_count;
            for (i = 0; i < byte_count; i++) {
                switch (*--bp) {
                case '\n':
                case '\r':
                    break;
                default:
                    trunc_size -= i;
                    /* to escape from both loops */
                    i = byte_count;
                    offset = 0;
                }
            }
        }

        /* buffer all newlines, truncate to at least here, but only if
         * not escaping from the loop */
        if (offset > 0) {
            trunc_size -= byte_count;
            offset -= byte_count;
        }
    }

    /* An edge case is if the file is changed between the "find the offset"
     * and this here truncate, but that gets into locking, or more properly
     * the file workflow and how to avoid locking, for which I am fond of
     * rename(2). */
    if (trunc_size < total_size)
        if (ftruncate(fd, trunc_size) == -1)
            err(EX_IOERR, "could not truncate '%s'", file);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: stripeofeol file [file2 ...]\n");
    exit(EX_USAGE);
}
