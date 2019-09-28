/* seek - `dd bs=1 [if=..] iseek=..` but without the dd-isms, or some
 * combination of GNU head and tail which typically are not installed */

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

/* this number is assumed by unit tests; update those if this changes */
#define BUFSIZE 8192

int Flag_Append = O_TRUNC;      /* -a */

char buf[BUFSIZE];

void consume(int ch, unsigned long seekto, char *input_file);
void emit_help(void);
void readthrough(int ch, unsigned long quitafter, char *input_file,
                 int outfd, char *output_file);

int main(int argc, char *argv[])
{
    char *input_file, *output_file = NULL;
    int ch, outfd;
    off_t fdeof, fdpos;
    unsigned long seekto, quitafter = ULONG_MAX;

#ifdef __OpenBSD__
    if (pledge("cpath rpath stdio wpath", NULL) == -1)
        err(1, "pledge failed");
#endif

    while ((ch = getopt(argc, argv, "ah?m:o:")) != -1) {
        switch (ch) {
        case 'a':
            Flag_Append = O_APPEND;
            break;
        case 'm':
            quitafter = flagtoul(ch, optarg, 1UL, ULONG_MAX);
            break;
        case 'o':
            output_file = optarg;
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

    if (argc < 1 || argc > 2)
        emit_help();

    seekto = argtoul("seek", *argv, 0UL, ULONG_MAX);
    argv++;

    if (argc == 1 || strncmp(*argv, "-", 2) == 0) {
        ch = STDIN_FILENO;
        input_file = "-";
    } else {
        if ((ch = open(*argv, O_RDONLY)) == -1)
            err(EX_IOERR, "could not open '%s'", *argv);
        input_file = *argv;
    }

    if (output_file == NULL || strncmp(output_file, "-", 2) == 0) {
        outfd = STDOUT_FILENO;
        output_file = "-";
    } else {
        if ((outfd =
             open(output_file, O_CREAT | O_WRONLY | Flag_Append, 0666)) == -1)
            err(EX_IOERR, "could not write '%s'", output_file);
    }

    /* fail if cannot seek to the desired location. this is complicated
     * by sparse file support that allows seeks to beyond the end of the
     * file and by the fact that the input may be unseekable */
    if ((fdeof = lseek(ch, (off_t) 0, SEEK_END)) == -1) {
        /* PORTABILITY there may be OS where other errors besides ESPIPE
         * can allow instead a consume() call... */
        if (errno == ESPIPE) {
            consume(ch, seekto, input_file);
        } else {
            err(EX_IOERR, "lseek failed (%d)", errno);
        }
    } else {                    /* seek to EOF did not fail, so ... */
        if ((fdpos = lseek(ch, (off_t) seekto, SEEK_SET)) == -1)
            err(EX_IOERR, "lseek failed (%d)", errno);
        if (fdpos > fdeof)
            errx(EX_IOERR, "could not seek");
    }

    readthrough(ch, quitafter, input_file, outfd, output_file);

    exit(EXIT_SUCCESS);
}

inline void consume(int ch, unsigned long seekto, char *input_file)
{
    ssize_t amount;
    size_t consume;
    long long seen = 0;

    while (1) {
        consume = BUFSIZE;
        if (BUFSIZE > seekto - seen) {
            consume = seekto - seen;
            if (consume == 0)
                break;
        }
        if ((amount = read(ch, buf, consume)) < 0)
            err(EX_IOERR, "read failed on '%s'", input_file);
        /* EOF is an error here to avoid the subsequent code doing
         * nothing with no error; also want to ensure can seek to where
         * the user asked, failing otherwise */
        if (amount == 0)
            errx(EX_IOERR, "could not seek");
        seen += amount;
    }
}

void emit_help(void)
{
    fputs("Usage: seek [-a] [-m max] [-o outfile] seekto [file|-]\n", stderr);
    exit(EX_USAGE);
}

inline void readthrough(int ch, unsigned long quitafter, char *input_file,
                        int outfd, char *output_file)
{
    ssize_t amount, written;
    size_t consume;
    long long seen = 0;

    while (1) {
        consume = BUFSIZE;
        if (BUFSIZE > quitafter - seen) {
            consume = quitafter - seen;
            if (consume == 0)
                break;
        }
        if ((amount = read(ch, buf, consume)) < 0)
            err(EX_IOERR, "read failed on '%s'", input_file);
        if (amount == 0)        /* EOF */
            break;
        if ((written = write(outfd, buf, amount)) < 0)
            err(EX_IOERR, "write failed on '%s'", output_file);
        if (written != amount)
            err(EX_IOERR, "incomplete write on '%s'", output_file);
        seen += amount;
    }
}
