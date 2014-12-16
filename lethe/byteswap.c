/*
 * Swaps bytes in a file (or files) at offsets given by the -a and -b
 * options. May cause corruption (that likely being the point), or
 * perhaps fail halfway though (meh?), depending on what goes awry with
 * I/O. Backups, as always, may be more prudent than not. One way to see
 * what byte swaps can cause would work something like:
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
 */

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

long Flag_OffA;                 /* -a */
long Flag_OffB;                 /* -b */

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, fd;
    char byte_a, byte_b, *ep;
    ssize_t bytes_read, bytes_written;

    while ((ch = getopt(argc, argv, "a:b:h")) != -1) {
        switch (ch) {
        case 'a':
	    Flag_OffA = strtol(optarg, &ep, 10);
	    if (optarg[0] == '\0' || *ep != '\0')
                errx(EX_DATAERR, "could not parse -a offset");
            if (Flag_OffA < 0 || Flag_OffA > INT_MAX - 1)
                errx(EX_DATAERR, "option -a out of range");
            break;

        case 'b':
	    Flag_OffB = strtol(optarg, &ep, 10);
	    if (optarg[0] == '\0' || *ep != '\0')
                errx(EX_DATAERR, "could not parse -b offset");
            if (Flag_OffB < 0 || Flag_OffB > INT_MAX - 1)
                errx(EX_DATAERR, "option -b out of range");
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

    if (argc == 0 || (Flag_OffA == Flag_OffB))
      emit_help();

    while (*argv) {
        if ((fd = open(*argv, O_RDWR)) == -1)
            err(EX_IOERR, "could not open '%s'", *argv);

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

        if (close(fd) == -1)
            err(EX_IOERR, "problem closing '%s'", *argv);

        argv++;
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: byteswap -a offsetn -b offsetm file [file1 ..]\n");
    exit(EX_USAGE);
}
