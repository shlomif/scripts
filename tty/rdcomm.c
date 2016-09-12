/*
 * Reads from a serial line, writes a subset of the ASCII seen to
 * standard output. Attempts to be smart about what device to read if
 * none is given, at least if on Mac OS X or OpenBSD. On OpenBSD, see
 * instead cu(1). Elsewhere, or for more bells and whistles, see
 * minicom(1). Any references to cu(1) in this code refer to the OpenBSD
 * version of that utility (as of OpenBSD 6.0).
 *
 * This code mostly to easily(?) read data via serial from an Arduino
 * and learn me some termios(4) stuff.
 *
 * To exit, kill the process or hit control+c or disrupt the serial line.
 */

#include <sys/ioctl.h>
#include <sys/types.h>

#include <err.h>
#include <fcntl.h>
#include <glob.h>
#include <paths.h>
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
bool Flag_Raw;                  // -r  raw, do not defang special ASCII chars

uint8_t serialdata[SERIAL_BUF_LEN];

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, device_fd;
    char *device = NULL;
    glob_t devglob;
    ssize_t readret;
    struct termios device_tio;

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
        /* Mac OS X - * in glob will be a bunch of digits, is created
         * on the fly; the tty.* device is read-only vs. the cu.* dev
         *   http://stackoverflow.com/questions/8632586
         */
        if (glob("/dev/tty.usbmodem*", GLOB_NOSORT, NULL, &devglob) == 0) {
            if (devglob.gl_pathc > 0) {
                if (devglob.gl_pathc > 1) {
                    warnx("notice: >1 devices found, picking one of them");
                }
                device = devglob.gl_pathv[0];
            }
        }
        /* OpenBSD - static USB dev, guess the first one (though
         * sometimes it may jump to cuaU1 after plug/unplug things...).
         * User must be in the 'dialer' group as of OpenBSD 5.8. */
        else if (glob("/dev/cuaU0*", 0, NULL, &devglob) == 0) {
            if (devglob.gl_pathc > 0) {
                device = devglob.gl_pathv[0];
            }
        }
        if (device) {
            warnx("info: using device '%s'", device);
        }
    } else {
        if (strchr(*argv, '/') == NULL) {
            if (asprintf(&device, "%s%s", _PATH_DEV, *argv) == -1)
                err(EX_OSERR, "could not asprintf() device path");
        } else {
            device = *argv;
        }
    }

    if (!device) {
        warnx("no device file found to read from");
        emit_help();
        /* NOTREACHED */
    }

    /* Need non-block here so open does not stall forever. Older unix
     * used O_NDELAY, but that "should not be used for new applications"
     * (APUE 2nd edition, p.442) */
    // TODO O_NOCTTY needed? cu(1) does not use it
    if ((device_fd = open(device, O_RDONLY | O_NOCTTY | O_NONBLOCK)) == -1)
        err(EX_IOERR, "could not open '%s'", device);

    // exclusive, no more open operations on terminal permitted
    ioctl(device_fd, TIOCEXCL);

    if (tcgetattr(device_fd, &device_tio) == -1)
        err(EX_IOERR, "tcgetattr() failed on '%s'", device);
    // see termios(4) though even those docs may not be clear on the
    // whys or whatfors of these various flags...
    device_tio.c_cc[VMIN] = 1;
    device_tio.c_cc[VTIME] = 0;
    device_tio.c_cflag &= ~(CSIZE | PARENB);
    device_tio.c_cflag |= CREAD | CS8 | CLOCAL;
    /* BREAK untested as cu(1) -> rdcomm via two serial lines (one a
     * "Prolific Technology Inc. USB-Serial Controller" and the other a
     * "FTDI FT232R USB UART") yielded NUL on ~# in cu, and unsure how
     * to send proper BREAK via arduino. */
    // NOTE may need option to unset or use different CR/NL flags, as
    // cu(1) to rdcomm needs ICRNL not set (no output), while arduino to
    // rdcomm does need it set (otherwise doubled newlines). Sigh.
    device_tio.c_iflag &= ~(ISTRIP | ICRNL | IGNBRK);
    device_tio.c_iflag |= BRKINT;
    device_tio.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);
    device_tio.c_oflag &= ~OPOST;
    cfsetspeed(&device_tio, (speed_t) Flag_Baud);
    if (tcsetattr(device_fd, TCSAFLUSH, &device_tio) == -1)
        err(EX_IOERR, "tcsetattr() failed on '%s'", device);

    // confirm settings were actually made (APUE does this...)
    if (tcgetattr(device_fd, &device_tio) == -1)
        err(EX_IOERR, "tcgetattr() failed on '%s'", device);
    if (device_tio.c_cc[VTIME] != 0 || device_tio.c_cc[VMIN] != 1 ||
        (device_tio.c_cflag & (CSIZE | PARENB | CREAD | CS8 | CLOCAL) !=
         (CREAD | CS8 | CLOCAL))
        || (device_tio.c_iflag & (ISTRIP | ICRNL | IGNBRK | BRKINT) != BRKINT)
        || (device_tio.c_lflag & (ICANON | ISIG | IEXTEN | ECHO))
        || device_tio.c_oflag & OPOST) {
        errx(EX_OSERR, "did not tcsetattr() properly");
     }

    fcntl(device_fd, F_SETFL, 0);       // back to blocking mode

    while (true) {
        if ((readret =
             read(device_fd, serialdata, (size_t) SERIAL_BUF_LEN)) == -1)
            err(EX_IOERR, "read failed on '%s' (%d)", device, errno);
        if (readret == 0)
            errx(EX_IOERR, "EOF on read of '%s'", device);

        if (!Flag_Raw) {
            for (ssize_t i = 0; i < readret; i++) {
                // defang del, not-ASCII high bit characters, and most of
                // the low ASCII control characters (especially ESC)
                if ((serialdata[i] < 32 && serialdata[i] != 9
                     && serialdata[i] != 10 && serialdata[i] != 13)
                    || serialdata[i] > 126) {
                    serialdata[i] = '.';
                }
            }
        }

        fwrite(serialdata, (size_t) readret, 1, stdout);
    }

    exit(1);                    /* NOTREACHED */
}

void emit_help(void)
{
    fprintf(stderr,
            "Usage: rdcomm [-B baud] [-l | -u] [-r] [/dev/something]\n");
    exit(EX_USAGE);
}
