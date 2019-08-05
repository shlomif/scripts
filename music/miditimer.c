/* miditimer - script to collect timing information on MIDI events.
 * assumes OpenBSD w/ MIDI attached, run via:
 *
 *   make miditimer
 *   ./miditime /dev/rmidi0 | tee results
 */

#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

/* nanoseconds is 10e-9 so nine zeros; msec is 10e-6 */
#define NSEC_IN_SEC  1000000000
#define NSEC_TO_MSEC 1000000

#define BUFSIZE 3
uint8_t buf[BUFSIZE];

const char *Default_Dev = "/dev/rmidi0";

int main(int argc, char *argv[])
{
    int fd;
    struct timespec when;       // , then;
    const char *mididev;
    ssize_t amount;

#ifdef __OpenBSD__
    if (pledge("rpath stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    mididev = (argv[1] == NULL) ? Default_Dev : argv[1];

    if ((fd = open(mididev, O_RDONLY)) == -1)
        err(EX_IOERR, "could not open MIDI device '%s'", mididev);

    setvbuf(stdout, (char *) NULL, _IOLBF, (size_t) 0);

    //then.tv_sec = ~0;

    while (1) {
        if ((amount = read(fd, &buf, (size_t) BUFSIZE)) < 1)
            errx(EX_IOERR, "read() returned %ld", amount);

        /* NOTE that longer messages (e.g. the ones with potentially
         * unlimited message bytes) are not supported, as they would
         * complicate parsing. fail if first byte not a status byte to
         * at least detect such */
        if ((buf[0] >> 7 & 1) != 1)
            errx(1, "unsupported not-status message in first byte");

        /* status or sync or something. skip */
        if (amount == 1 && buf[0] == 0xfe)
            continue;

        if (clock_gettime(CLOCK_MONOTONIC, &when) == -1)
            err(EX_OSERR, "clock_gettime() failed");

        printf("%lu.%03ld ",
               (unsigned long) when.tv_sec, when.tv_nsec / NSEC_TO_MSEC);
        for (ssize_t i = 0; i < amount; i++) {
            printf("%2x", buf[i]);
        }
        //if (then.tv_sec != ~0) {
        // TODO print delta time
        //}
        putchar('\n');
        //then.tv_sec = when.tv_sec;
        //then.tv_nsec = when.tv_nsec;
    }
    exit(1);                    /* *NOTREACHED* */
}
