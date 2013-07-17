/*
 * Snips any ultimate linefeed chars (\r and \n) from the named files.
 *
 * This is mostly just silly programming practice; files on Unix often
 * need that ultimate newline, as otherwise shell 'while' loops might
 * loose that last line, and so forth. (Hence certain editors warning if
 * the file lacks a trailing newline.)
 */

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

void trim_file(const int fd, const char *file);
void usage(void);

int main(int argc, char *argv[])
{
    int ch, fd;

    while ((ch = getopt(argc, argv, "h?")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
            usage();
            /* NOTREACHED */
        default:
            ;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 1)
        usage();

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

/*
 * Gist of logic is to step backwards through the file, ending
 * when a non-newline character is detected, and then truncating
 * the file at that point. This will be quick for normal files,
 * but less so for pathological cases of large files consisting
 * entirely of newlines. (For which a larger buf and looping over
 * that would help avoid excess system calls.)
 */
void trim_file(const int fd, const char *file)
{
    char buf;
    long start_size, trunc_size, offset;

    if ((start_size = lseek(fd, 0, SEEK_END)) < 0)
        err(EX_IOERR, "could not seek '%s'", file);
    if (start_size == 0)
        return;                 /* empty file, whatevs */

    trunc_size = start_size;

    for (offset = start_size; offset > 0; offset--) {
        if (pread(fd, &buf, 1, offset - 1) != 1)
            err(EX_IOERR, "could not pread '%s'", file);

        switch (buf) {
        case '\n':
        case '\r':
            break;
        default:
            /* non-newline, stop at this offset */
            goto TEN;
        }
    }
  TEN:
    trunc_size = offset;

    /*
     * An edge case is if the file is changed between the "find the
     * offset code" and this here truncate, but that gets into locking,
     * or more properly the file workflow and how to avoid locking, for
     * which I am fond of rename(2).
     */
    if (trunc_size < start_size)
        if (ftruncate(fd, trunc_size) == -1)
            err(EX_IOERR, "could not truncate '%s'", file);
}

void usage(void)
{
    errx(EX_USAGE, "need files to snip ultimate newlines from");
}
