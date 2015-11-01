/*
 * Swaps bytes in a file (or files) at offsets given by the -a and -b
 * options (or randomly, maybe, with -r). May cause corruption (that
 * likely being the point), or perhaps fail halfway though (meh?),
 * depending on what goes awry with I/O. Backups, as always, may be more
 * prudent than not. One way to see what byte swaps can cause would work
 * something like:
 * 
 *   #!/usr/bin/env zsh
 *
 *   # dotted so not globbed by cleanup
 *   SRC_IMG=.src.jpg
 *
 *   # extirpate any from previous run
 *   perl -e 'unlink glob "*.jpg"'
 *
 *   # though only at i>100 did things get interesting for the sample
 *   for i in {1..9999}; do
 *       for j in {$((i+1))..$((i+99))}; do
 *           cp $SRC_IMG $i-$j.jpg
 *           ./byteswap -a $i -b $j $i-$j.jpg;
 *       done
 *   done
 *
 * Plus perhaps some means to eliminate images obviously corrupt or
 * otherwise too similar or not interesting enough. This tool appears to
 * cause much more damage and variety of damage than done by
 * byteswapprime.c, and could easily be performed on the raw bitmap in
 * memory to avoid excess I/O in a workflow.
 *
 * The "Ship of Theseus" paradox likely applies to the thus reordered yet
 * unchanged sets of bytes.
 *
 * With -r [0..100] the offsets will be determined randomly, and the
 * swap only done if a random number from 0..99 is less than the
 * argument to -r. Thus `-r 100` will always swap two bytes (assuming
 * the file is large enough to support that), while `-r 50` would do so
 * only about half the time.
 */

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

// https://github.com/thrig/libjkiss
#include <jkiss.h>

long long Flag_OffA;            /* -a */
long long Flag_OffB;            /* -b */
unsigned long Flag_Rand;        /* -r */

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, fd;
    char byte_a, byte_b;
    ssize_t bytes_read, bytes_written;
    off_t filesize;

    while ((ch = getopt(argc, argv, "a:b:hr:")) != -1) {
        switch (ch) {
        case 'a':
            Flag_OffA =
                (long long) flagtoul(ch, optarg, 0UL,
                                     (unsigned long) LLONG_MAX);
            break;

        case 'b':
            Flag_OffB =
                (long long) flagtoul(ch, optarg, 0UL,
                                     (unsigned long) LLONG_MAX);
            break;

        case 'r':
            Flag_Rand = flagtoul(ch, optarg, 0UL, 100UL);
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

    if (!Flag_Rand && Flag_OffA == Flag_OffB)
        errx(EX_DATAERR, "-a equals -b, nothing to do!");

    jkiss64_init(NULL);

    while (*argv) {
        if ((fd = open(*argv, O_RDWR)) == -1)
            err(EX_IOERR, "could not open '%s' for O_RDWR", *argv);

        if (Flag_Rand > 0) {
            if (jkiss64_uniform(100UL) < Flag_Rand) {
                if ((filesize = lseek(fd, 0, SEEK_END)) == -1)
                    err(EX_IOERR, "could not lseek '%s'", *argv);
                if (filesize < 2)
                    errx(EX_DATAERR, "file '%s' too small to swap bytes",
                         *argv);
                Flag_OffA = Flag_OffB =
                    (off_t) jkiss64_uniform((uint64_t) filesize);
                while (Flag_OffA == Flag_OffB) {
                    Flag_OffB = (off_t) jkiss64_uniform((uint64_t) filesize);
                }
                fprintf(stderr, "dbg do swap %lld %lld (fs %lld)\n", Flag_OffA,
                        Flag_OffB, filesize);
            } else {
                goto NEXTFILE;
            }
        }

        bytes_read = pread(fd, &byte_a, (size_t) 1, Flag_OffA);
        if (bytes_read == -1)
            err(EX_IOERR, "A pread() failure on '%s'", *argv);
        else if (bytes_read == 0)
            errx(EX_IOERR, "A read of '%s': EOF", *argv);
        else if (bytes_read != 1)
            errx(EX_IOERR, "A unexpected pread() on '%s'", *argv);

        bytes_read = pread(fd, &byte_b, (size_t) 1, Flag_OffB);
        if (bytes_read == -1)
            err(EX_IOERR, "B pread() failure on '%s'", *argv);
        else if (bytes_read == 0)
            errx(EX_IOERR, "B read of '%s': EOF", *argv);
        else if (bytes_read != 1)
            errx(EX_IOERR, "B unexpected pread() on '%s'", *argv);

        bytes_written = pwrite(fd, &byte_a, (size_t) 1, Flag_OffB);
        if (bytes_written == -1)
            err(EX_IOERR, "A pwrite() failure on '%s'", *argv);
        else if (bytes_written != 1)
            errx(EX_IOERR, "A unequal pwrite() on '%s'", *argv);

        bytes_written = pwrite(fd, &byte_b, (size_t) 1, Flag_OffA);
        if (bytes_written == -1)
            err(EX_IOERR, "B pwrite() failure on '%s'", *argv);
        else if (bytes_written != 1)
            errx(EX_IOERR, "B unequal pwrite() on '%s'", *argv);

      NEXTFILE:
        if (close(fd) == -1)
            err(EX_IOERR, "problem closing '%s'", *argv);

        argv++;
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr,
            "Usage: byteswap [-a offsetn -b offsetm|-r odds] file [file1 ..]\n");
    exit(EX_USAGE);
}
