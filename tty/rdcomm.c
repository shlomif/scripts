/*
 * Reads from a serial line, writes a subset of the ASCII seen to
 * standard output. Attempts to be smart about what device to read if
 * none is given, at least if on Mac OS X or OpenBSD. On OpenBSD, see
 * instead cu(1). Elsewhere, or for more bells and whistles, see
 * minicom(1). Any references to cu(1) in this code refer to the OpenBSD
 * version of that utility (as of OpenBSD 6.0).
 *
 * This code mostly to easily(?) read data via serial from an Arduino
 * and learn me some termios(4) (and TCL) stuff.
 *
 * To exit, kill the process or hit control+c or disrupt the serial line.
 */

#include <sys/ioctl.h>
#include <sys/types.h>

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <glob.h>
#include <paths.h>
#include <pwd.h>
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

#include <tcl.h>

#define BAUD_MAX 640 * 640
#define SERIAL_BUF_LEN 80

speed_t Flag_Baud = B9600;      // -B  baud
bool Flag_CR;                   // -C  do not ignore CR
uint8_t Flag_Bits;              // -I  bits (5-8)
unsigned long Flag_MinRead = 1; // -M  for VMIN read
uint8_t Flag_Parity;            // -P  parity (0-2 off odd even)
bool Flag_Raw;                  // -r  raw, do not defang special ASCII chars
bool Flag_StopBits;             // -S  two stop bits if set

void emit_help(void);
void set_dev_termios(const int fd, const char *device);

int main(int argc, char *argv[])
{
    int ch, device_fd;
    char *device = NULL;
    const char *homedir;
    char *serialdata, *tclshrc;
    glob_t devglob;
    ssize_t readret;
    Tcl_Interp *Interp = NULL;
    Tcl_Obj *Script = NULL;

    while ((ch = getopt(argc, argv, "B:Ce:h?I:M:P:rS")) != -1) {
        switch (ch) {
        case 'B':
            Flag_Baud =
                (speed_t) flagtoul(ch, optarg, 1UL, (unsigned long) BAUD_MAX);
            break;
        case 'C':
            Flag_CR = true;
            break;
        case 'e':
            Script = Tcl_NewStringObj(optarg, -1);
            Tcl_IncrRefCount(Script);
            break;
        case 'I':
            Flag_Bits = (uint8_t) flagtoul(ch, optarg, 5UL, 8UL);
            break;
        case 'M':
            Flag_MinRead =
                flagtoul(ch, optarg, 1UL, (unsigned long) SERIAL_BUF_LEN);
            break;
        case 'P':
            Flag_Parity = (uint8_t) flagtoul(ch, optarg, 0UL, 2UL);
            break;
        case 'r':
            Flag_Raw = true;
            break;
        case 'S':
            Flag_StopBits = true;
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

    if ((serialdata = malloc(sizeof(char) * (SERIAL_BUF_LEN + 1))) == NULL)
        err(EX_OSERR, "could not malloc() serial data buffer");

    if (Script) {
        int ret;
        Interp = Tcl_CreateInterp();
        if (Tcl_PkgRequire(Interp, "Tcl", "8.5", 0) == NULL)
            errx(1, "need TCL >= 8.5");

        if ((homedir = getenv("HOME")) == NULL) {
            homedir = getpwuid(getuid())->pw_dir;
            if (!homedir)
                errx(EX_OSERR, "could not determine HOME directory");
        }
        if (asprintf(&tclshrc, "%s/%s", homedir, ".tclshrc") == -1)
            err(EX_OSERR, "could not asprintf() tclshrc path");
        /* TODO better way to ignore rc file missing? */
        if ((ret = Tcl_EvalFile(Interp, tclshrc)) != TCL_OK) {
            char *msg;
            int len;
            Tcl_Obj *options = Tcl_GetReturnOptions(Interp, ret);
            Tcl_Obj *key = Tcl_NewStringObj("-errorcode", -1);
            Tcl_Obj *errcode;
            Tcl_IncrRefCount(key);
            Tcl_DictObjGet(NULL, options, key, &errcode);
            Tcl_DecrRefCount(key);
            msg = Tcl_GetStringFromObj(errcode, &len);
            if (len > 12)
                len = 12;
            if (strncmp(msg, "POSIX ENOENT", len) != 0)
                errx(1, "TCL error: %s", Tcl_GetStringResult(Interp));
        }

        Tcl_LinkVar(Interp, "_", (void *) &serialdata,
                    TCL_LINK_STRING | TCL_LINK_READ_ONLY);
    }

    if (argc == 0 || *argv == NULL) {
        /* Mac OS X - * in glob will be a bunch of digits, is created
         * on the fly; the tty.* device is read-only vs. the cu.* dev
         *   http://stackoverflow.com/questions/8632586
         */
        if (glob("/dev/tty.usbmodem*", GLOB_NOSORT, NULL, &devglob) == 0) {
            if (devglob.gl_pathc > 0) {
                if (devglob.gl_pathc > 1)
                    warnx("notice: >1 devices found, picking one of them");
                device = devglob.gl_pathv[0];
            }
        }
        /* OpenBSD - static USB dev, guess the first one (though
         * sometimes it may jump to cuaU1 after plug/unplug things...).
         * User must be in the 'dialer' group as of OpenBSD 5.8. */
        else if (glob("/dev/cuaU0", 0, NULL, &devglob) == 0) {
            if (devglob.gl_pathc > 0)
                device = devglob.gl_pathv[0];
        }
        if (device)
            warnx("info: using device '%s'", device);
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

    /* Need non-block here so open does not stall forever. Olden unix
     * used O_NDELAY, but that "should not be used for new applications"
     * (APUE 2nd edition, p.442) */
    // TODO O_NOCTTY needed? cu(1) does not use it
    if ((device_fd = open(device, O_RDONLY | O_NOCTTY | O_NONBLOCK)) == -1)
        err(EX_IOERR, "could not open '%s'", device);

    // exclusive, no more open operations on terminal permitted?
    ioctl(device_fd, TIOCEXCL);

    set_dev_termios(device_fd, device);

    fcntl(device_fd, F_SETFL, 0);       // back to blocking mode

    while (true) {
        if ((readret =
             read(device_fd, serialdata, (size_t) SERIAL_BUF_LEN)) == -1)
            err(EX_IOERR, "read failed on '%s' (%d)", device, errno);
        if (readret == 0) {
            errx(EX_IOERR, "EOF on read of '%s'", device);
        } else if (readret < 0) {
            errx(1, "unexpected negative return from read: %ld", readret);
        }

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

        if (Script) {
            serialdata[readret] = '\0';
            if (Tcl_EvalObjEx(Interp, Script, TCL_EVAL_GLOBAL) != TCL_OK)
                errx(1, "TCL error: %s", Tcl_GetStringResult(Interp));
        } else {
            write(STDOUT_FILENO, serialdata, (size_t) readret);
        }
    }

    exit(1);                    /* NOTREACHED */
}

void emit_help(void)
{
    fprintf(stderr,
            "Usage: rdcomm [-B baud] [-e expr] [-M minread] [-r] [/dev/foo]\n");
    exit(EX_USAGE);
}

/* see termios(4) though even those docs may not be clear on the whys or
 * whatfors of these various flags... */
void set_dev_termios(const int fd, const char *device)
{
    struct termios tio;
    tcflag_t cflags, iflags;

    if (tcgetattr(fd, &tio) == -1)
        err(EX_IOERR, "tcgetattr() failed on '%s'", device);

    cfsetspeed(&tio, Flag_Baud);

    /* block until read VMIN due to timer being disabled */
    tio.c_cc[VMIN] = Flag_MinRead;
    tio.c_cc[VTIME] = 0;

    /************************* Control Modes *************************/
    cflags = tio.c_cflag;
    cflags |= (CLOCAL | CREAD);

    cflags &= ~CSIZE;
    switch (Flag_Bits) {
    case 5:
        cflags |= CS5;
        break;
    case 6:
        cflags |= CS6;
        break;
    case 7:
        cflags |= CS7;
        break;
    default:
        cflags |= CS8;
    }

    // see also how parity is handled in input modes section
    switch (Flag_Parity) {
    case 1:
        cflags |= PARODD;
        // no break as fall through to enable parity
    case 2:
        cflags |= PARENB;
        break;
    default:
        cflags &= ~(PARENB);
    }

    if (Flag_StopBits) {
        cflags |= CSTOPB;
    }

    tio.c_cflag = cflags;

    /************************* Input Modes *************************/
    iflags = tio.c_iflag;

    // NOTE BREAK is untested, so these BRK settings may not be desireable
    iflags &= ~(ICRNL | IGNBRK | IMAXBEL | INLCR | INPCK | ISTRIP);
    iflags |= (BRKINT | IGNPAR);

    if (Flag_CR) {
        iflags &= ~IGNCR;
    } else {
        iflags |= IGNCR;
    }

    if (Flag_Parity) {
        /* NOTE parity errors untested; may need flag to let user pick
         * between marking and discarding any such errors. For now mark
         * them to draw attention. */
        iflags |= (INPCK | PARMRK);
        iflags &= ~IGNPAR;
    }

    tio.c_iflag = iflags;

    /************************* Local Modes *************************/
    tio.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);

    /************************* Output Modes *************************/
    tio.c_oflag &= ~OPOST;

    if (tcsetattr(fd, TCSAFLUSH, &tio) == -1)
        err(EX_IOERR, "tcsetattr() failed on '%s'", device);

    // confirm settings were actually made (APUE does this...)
    if (tcgetattr(fd, &tio) == -1)
        err(EX_IOERR, "tcgetattr() failed after set on '%s'", device);
    if (tio.c_cc[VTIME] != 0 || tio.c_cc[VMIN] != Flag_MinRead
        || tio.c_cflag != cflags || tio.c_iflag != iflags
        || (tio.c_lflag & (ICANON | ISIG | IEXTEN | ECHO))
        || tio.c_oflag & OPOST) {
        errx(EX_OSERR, "did not tcsetattr() properly??");
    }
}
