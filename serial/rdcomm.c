/*
 * Reads from a serial line, writes a subset of the ASCII seen to
 * STDOUT. Attempts to be smart about what device to read if none is
 * given, at least if on Mac OS X or OpenBSD. For more bells and
 * whistles, try minicom(1). Assuming you remembered to install that.
 * (Or, pro tip(1): use cu(1) instead, given that tip(1) is gone now.)
 *
 * (Written mostly for serial practice, and to easily read output
 * from an Arduino.)
 *
 * Untested: what happens if serial line sends BREAK.
 */

#include <sys/types.h>

#include <err.h>
#include <fcntl.h>
#include <glob.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <termios.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#define BAUD_MAX 640 * 640
#define SERIAL_BUF_LEN 80

unsigned long Flag_Baud = B9600;
bool Flag_Raw;                  // -r

uint8_t serialdata[SERIAL_BUF_LEN];
struct termios Term_Options;

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, fd;
    const char *device = NULL;
    glob_t devglob;
    ssize_t readret;

    while ((ch = getopt(argc, argv, "B:h?lru")) != -1) {
        switch (ch) {
        case 'B':
            Flag_Baud = flagtoul(ch, optarg, 1UL, (unsigned long) BAUD_MAX);
            break;

        case 'l':
            setvbuf(stdout, (char *) NULL, _IOLBF, (size_t) 0);
            break;

        case 'r':
            Flag_Raw = true;
            break;

        case 'u':
            setvbuf(stdout, (char *) NULL, _IONBF, (size_t) 0);
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

    if (argc == 0 || *argv == NULL) {
        // Mac OS X
        if (glob("/dev/tty.usbmodem*", GLOB_NOSORT, NULL, &devglob) == 0) {
            if (devglob.gl_pathc > 0) {
                if (devglob.gl_pathc > 1)
                    warnx("notice: >1 devices found, picking one of them");
                device = devglob.gl_pathv[0];
                warnx("info: using device '%s'", device);
            }
        }
        /* OpenBSD - USB dev, guess the first one (though sometimes it
         * may jump to cuaU1 after plug/unplug things...). User will
         * need to be in the 'dialer' group as of OpenBSD 5.8. */
        if (glob("/dev/cuaU0*", GLOB_NOSORT, NULL, &devglob) == 0) {
            if (devglob.gl_pathc > 0) {
                device = devglob.gl_pathv[0];
                warnx("info: using device '%s'", device);
            }
        }
    } else {
        device = *argv;
    }

    if (!device)
        errx(EX_USAGE, "no device file found to read from");

    /* Need non-block here so open does not stall forever. Older unix
     * used O_NDELAY, but that "should not be used for new applications"
     * (APUE 2nd edition, p.442) */
    if ((fd = open(device, O_RDONLY | O_NOCTTY | O_NONBLOCK)) == -1)
        err(EX_IOERR, "could not open '%s'", device);

    /* TODO more modern way to do this (without this get "Resource
     * temporarily unavailable" read failures) - probably via
     * poll(2) or such, but that's more complicated. */
    fcntl(fd, F_SETFL, 0);

    if (tcgetattr(fd, &Term_Options) == -1)
        err(EX_IOERR, "tcgetattr() failed on '%s'", device);

    cfsetispeed(&Term_Options, (speed_t) Flag_Baud);
    cfsetospeed(&Term_Options, (speed_t) Flag_Baud);
    // ignore modem status lines, enable receiver
    Term_Options.c_cflag |= (CLOCAL | CREAD);

    if (tcsetattr(fd, TCSANOW, &Term_Options) == -1)
        err(EX_IOERR, "tcsetattr() failed on '%s'", device);

    // TODO check that the settings actually took, as tcsetattr can
    // return 0 but not actually have made all the desired changes :/

    while (true) {
        if ((readret = read(fd, serialdata, (size_t) SERIAL_BUF_LEN)) == -1)
            err(EX_IOERR, "read failed on '%s' (%d)", device, errno);
        if (readret == 0)
            errx(1, "EOF on read of '%s'", device);

        if (!Flag_Raw) {
            for (ssize_t i = 0; i < readret; i++) {
                // defang del, not-ASCII high bit characters, and most of
                // the low ASCII control characters (especially ESC)
                if ((serialdata[i] < 32 && serialdata[i] != 9
                     && serialdata[i] != 10 && serialdata[i] != 13)
                    || serialdata[i] > 126)
                    serialdata[i] = '.';
            }
        }
        write(STDOUT_FILENO, serialdata, (size_t) readret);
    }

    exit(1);                    /* NOTREACHED */
}

void emit_help(void)
{
    fprintf(stderr,
            "Usage: rdcomm [-B baud] [-l | -u] [-r] [/dev/something]\n");
    exit(EX_USAGE);
}
