/* Script to collect timing information on MIDI pedal events. Assumes
 * OpenBSD w/ MIDI attached, run via:
 *
 *     make pedaltime
 *     ./pedaltime /dev/rmidi0 | tee results
 *
 * This was motivated by lilypond N\sustainOff\sustainOn half-pedals
 * being marked as a simultaneous event in the MIDI, which is not
 * physically possible. So how long does a half-pedal actually take?
 */

#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#define NSEC_IN_SEC  1000000000
#define NSEC_IN_MSEC 1000000

#define BUFSIZE 3
uint8_t buf[BUFSIZE];

int main(int argc, char *argv[])
{
    int fd, ret;
    struct timespec when;

    if ((fd = open(argv[1], O_RDONLY)) == -1)
        err(EX_IOERR, "could not open '%s'", argv[1]);

    setvbuf(stdout, (char *) NULL, _IOLBF, (size_t) 0);

    while (1) {
        if ((ret = read(fd, &buf, (size_t) BUFSIZE)) < 1)
            errx(EX_IOERR, "read() returned %d", ret);
        // too short a message
        if (ret < 3)
            continue;
        // not a control_change involving Damper Pedal (Sustain)
        if (buf[0] != 0xB0 || buf[1] != 0x40)
            continue;
        if (clock_gettime(CLOCK_MONOTONIC, &when) == -1)
            err(EX_OSERR, "clock_gettime() failed");
        // [Data Byte of 0-63=0ff, 64-127=On]
        printf("%lu.%03ld %s\n",
               (unsigned long) when.tv_sec, when.tv_nsec / NSEC_IN_MSEC,
               (buf[2] > 63) ? "on" : "off");
    }

    exit(1);                    // *NOTREACHED*
}
