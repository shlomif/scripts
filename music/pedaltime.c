/* Script to collect timing information on MIDI pedal events. Assumes
 * OpenBSD w/ MIDI attached, run via:
 *
 *     make pedaltime
 *     ./pedaltime /dev/rmidi0 | tee results
 *
 * This was motivated by lilypond N\sustainOff\sustainOn half-pedals
 * being marked as a simultaneous event in the MIDI, which is not
 * physically possible. So how long does a half-pedal actually take?
 *
 * (MIDI is (mostly) little-endian; this code has only been run
 * on a little-endian system so there may be portability issues on
 * big-endian systems.)
 */

#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

// nanoseconds is 10e-9 so nine zeros; msec is 10e-6
#define NSEC_IN_SEC  1000000000
#define NSEC_TO_MSEC 1000000

#define BUFSIZE 3
uint8_t buf[BUFSIZE];

const char *Default_Dev = "/dev/rmidi0";

int main(int argc, char *argv[])
{
    int fd, ret;
    struct timespec when;
    const char *mididev;

    argc--;
    argv++;
    mididev = (*argv == NULL) ? Default_Dev : *argv;

    if ((fd = open(mididev, O_RDONLY)) == -1)
        err(EX_IOERR, "could not open MIDI device '%s'", mididev);

    setvbuf(stdout, (char *) NULL, _IOLBF, (size_t) 0);

    while (1) {
        if ((ret = read(fd, &buf, (size_t) BUFSIZE)) < 1)
            errx(EX_IOERR, "read() returned %d", ret);

        // too short a message
        if (ret < 3)
            continue;

        // NOTE that longer messages (e.g. the ones with potentially
        // unlimited message bytes) are not supported, as they would
        // complicate parsing. Fail if first byte not a status byte
        // to at least detect such.
        if ((buf[0] >> 7 & 1) != 1)
            errx(1, "unsupported not-status message in first byte");

        // There's 16 channels for control/mode changes, 0b1011____
        // with the leading 1 being the status byte. 0x40 is the
        // damper (sustain) pedal indication.
        if ((buf[0] >> 4 & 0xF) != 11 || buf[1] != 0x40)
            continue;

        if (clock_gettime(CLOCK_MONOTONIC, &when) == -1)
            err(EX_OSERR, "clock_gettime() failed");

        // [Data Byte of 0-63=0ff, 64-127=On]
        printf("%lu.%03ld %s\n",
               (unsigned long) when.tv_sec, when.tv_nsec / NSEC_TO_MSEC,
               (buf[2] > 63) ? "on" : "off");
    }

    exit(1);                    // *NOTREACHED*
}

/*
 * (Answer: it depends! Skill of pianist and the design of the pedal are
 * major influences of how quickly a half-pedal can be performed.)
 */
