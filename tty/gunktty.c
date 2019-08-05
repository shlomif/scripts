/* gunktty - randomize TTY settings and optionally run something else */

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <sysexits.h>
#include <termios.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

int Flag_OpenTTY;               /* -T */

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, fd;
    struct termios tp;

#ifdef __OpenBSD__
    if (pledge("exec stdio tty", NULL) == -1)
        err(1, "pledge failed");
#endif

    fd = STDIN_FILENO;

    while ((ch = getopt(argc, argv, "h?d:T")) != -1) {
        switch (ch) {
        case 'd':
            fd = (int) flagtoul(ch, optarg, 0UL,
                                (unsigned long) getdtablesize());
            break;
        case 'T':
            Flag_OpenTTY = 1;
            break;
        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }

    if (Flag_OpenTTY) {
        if ((fd = open("/dev/tty", O_RDWR)) < 0)
            err(EX_OSERR, "could not open /dev/tty");
    }

    if (!isatty(fd))
        errx(1, "fd %d is not a TTY", fd);

    while (1) {
        arc4random_buf(&tp, sizeof(struct termios));
        /* this may fail on invalid inputs so KLUGE loop until not */
        if (tcsetattr(fd, TCSANOW, &tp) != -1)
            break;
    }

    if (argc > 0) {
        execvp(argv[1], &argv[1]);
        err(1, "could not exec '%s'", argv[1]);
    }
    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fputs("Usage: gunktty [-d fdnum | -T] -- [cmd [arg ..]]\n", stderr);
    exit(EX_USAGE);
}

/* P.S. if this breaks your terminal that is the point of this script */

/* P.P.S. don't say I didn't warn you */
