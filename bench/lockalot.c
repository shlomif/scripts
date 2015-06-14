/*
 * Locks memory in memory. Presumably for testing purposes of some sort,
 * e.g. to more readily run a system towards using swap, or such. This
 * program will most likely run into various limitations on memory locking,
 * see mlock(2) and the OS guide for how to change any such limits.
 *
 * -m for amount of memory to grab; with -L, does not lock, in which case
 * only memory is grabbed until something causes the process to terminate.
 */

#include <sys/mman.h>

#include <err.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#define MOSTMEMPOSSIBLE ( (ULONG_MAX < SIZE_MAX) ? ULONG_MAX : SIZE_MAX )

bool Flag_Skip_Mlock;           // -L
unsigned long Flag_Mem_Amount;  // -m

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    char *stuff;

    char tmp_filename[] = "/tmp/lockalot.XXXXXXXXXX";
    struct pollfd pfd[1];

    while ((ch = getopt(argc, argv, "h?Lm:")) != -1) {
        switch (ch) {

        case 'L':
            Flag_Skip_Mlock = true;

        case 'm':
            Flag_Mem_Amount = flagtoul(ch, optarg, 1UL, MOSTMEMPOSSIBLE);
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

    if (Flag_Mem_Amount == 0)
        emit_help();

    if ((stuff = malloc((size_t) Flag_Mem_Amount)) == NULL)
        err(EX_OSERR, "could not malloc() %lu bytes of memory",
            Flag_Mem_Amount);

    if (!Flag_Skip_Mlock)
        if (mlock(stuff, (size_t) Flag_Mem_Amount) == -1)
            err(EX_OSERR, "could not mlock() %lu bytes of memory",
                Flag_Mem_Amount);

    // Avoid busy loop if cannot block on input (e.g. started in background)
    if (isatty(STDIN_FILENO)) {
        pfd[0].fd = STDIN_FILENO;
    } else {
        fprintf(stderr, "notice: doing mkstemp to create file to poll...\n");
        if ((pfd[0].fd = mkstemp(tmp_filename)) == -1)
            err(EX_IOERR, "mkstemp failed to create tmp file");
    }
    pfd[0].events = POLLPRI;
    for (;;) {
        poll(pfd, 1, 60 * 10000);
    }

    /* NOTREACHED */
    exit(1);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: lockalot [-L] -m bytestolock\n");
    exit(EX_USAGE);
}
