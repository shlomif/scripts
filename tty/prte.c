/* prte - try to preserve terminal settings across a given command */

#include <sys/wait.h>

#include <err.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <termios.h>
#include <unistd.h>

#define LINELENGTH 72

enum { NOPE = -1, CHILD };

struct termios Original_Termios;

int Flag_Warn;                  /* -w */
int Flag_WarnVerbose;           /* -W */

int Col_Offset;

void cleanup(void);
void mention(char *s, int len);
void emit_help(void);
void tdelta(struct termios *a, struct termios *b);

int main(int argc, char *argv[])
{
    int ch, status;
    pid_t pid;

#ifdef __OpenBSD__
    if (pledge("exec proc stdio tty", NULL) == -1)
        err(1, "pledge failed");
#endif

    while ((ch = getopt(argc, argv, "h?Ww")) != -1) {
        switch (ch) {
        case 'W':
            Flag_WarnVerbose = 1;
        case 'w':
            Flag_Warn = 1;
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
    if (argc < 1)
        emit_help();

    if (tcgetattr(STDIN_FILENO, &Original_Termios) < 0)
        err(EX_OSERR, "tcgetattr failed");

    if ((pid = fork()) == NOPE)
        err(EX_OSERR, "fork failed");

    if (pid == CHILD) {
        execvp(argv[0], &argv[0]);
        err(1, "could not exec '%s'", argv[0]);
    }
    if (waitpid(pid, &status, 0) < 0)
        err(EX_OSERR, "waitpid failed");
    cleanup();
    if (WIFEXITED(status)) {
        exit(WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        signal(WTERMSIG(status), SIG_DFL);
        raise(WTERMSIG(status));
    }
    exit(1);
}

inline void cleanup(void)
{
    struct termios current;
    if (Flag_Warn) {
        if (tcgetattr(STDIN_FILENO, &current) < 0)
            err(EX_OSERR, "tcgetattr failed");
        if (memcmp(&Original_Termios, &current, sizeof(struct termios)) != 0) {
            if (Flag_WarnVerbose) {
                tdelta(&Original_Termios, &current);
            } else {
                warnx("termios differs");
            }
        }
    }
    if (tcsetattr(STDIN_FILENO, TCSANOW, &Original_Termios) < 0)
        err(EX_OSERR, "tcsetattr failed");
}

void emit_help(void)
{
    fputs("Usage: prte [-w | -W] -- command [arg ..]\n", stderr);
    exit(EX_USAGE);
}

/* adapted from bin/stty/print.c on OpenBSD 6.5 */
#define flcmp(field, flag, msg, len) \
    if ((a->field & flag) != (b->field & flag)) mention(msg, len)

/* NOTE this is for OpenBSD termios(4) so may not be portable elsewhere */
inline void tdelta(struct termios *a, struct termios *b)
{
    flcmp(c_lflag, ICANON, "icanon", 6);
    flcmp(c_lflag, ISIG, "isig", 4);
    flcmp(c_lflag, IEXTEN, "iexten", 6);
    flcmp(c_lflag, ECHO, "echo", 4);
    flcmp(c_lflag, ECHOE, "echoe", 5);
    flcmp(c_lflag, ECHOK, "echok", 5);
    flcmp(c_lflag, ECHOKE, "echoke", 6);
    flcmp(c_lflag, ECHONL, "echonl", 6);
    flcmp(c_lflag, ECHOCTL, "echoctl", 7);
    flcmp(c_lflag, ECHOPRT, "echoprt", 7);
    flcmp(c_lflag, ALTWERASE, "altwerase", 9);
    flcmp(c_lflag, NOFLSH, "noflsh", 6);
    flcmp(c_lflag, TOSTOP, "tostop", 6);
    flcmp(c_lflag, FLUSHO, "flusho", 6);
    flcmp(c_lflag, PENDIN, "pendin", 6);
    flcmp(c_lflag, NOKERNINFO, "nokerninfo", 10);
    flcmp(c_lflag, EXTPROC, "extproc", 7);
#ifdef __OpenBSD__
    flcmp(c_lflag, XCASE, "xcase", 5);
#endif

    flcmp(c_iflag, ISTRIP, "istrip", 6);
    flcmp(c_iflag, ICRNL, "icrnl", 5);
    flcmp(c_iflag, INLCR, "inlcr", 5);
    flcmp(c_iflag, IGNCR, "igncr", 5);
#ifdef __OpenBSD__
    flcmp(c_iflag, IUCLC, "iuclc", 5);
#endif
    flcmp(c_iflag, IXON, "ixon", 4);
    flcmp(c_iflag, IXOFF, "ixoff", 5);
    flcmp(c_iflag, IXANY, "ixany", 5);
    flcmp(c_iflag, IMAXBEL, "imaxbel", 7);
    flcmp(c_iflag, IGNBRK, "ignbrk", 6);
    flcmp(c_iflag, BRKINT, "brkint", 6);
    flcmp(c_iflag, INPCK, "inpck", 5);
    flcmp(c_iflag, IGNPAR, "ignpar", 6);
    flcmp(c_iflag, PARMRK, "parmrk", 6);

    flcmp(c_oflag, OPOST, "opost", 5);
    flcmp(c_oflag, ONLCR, "onlcr", 5);
    flcmp(c_oflag, OCRNL, "ocrnl", 5);
    flcmp(c_oflag, ONOCR, "onocr", 5);
    flcmp(c_oflag, ONLRET, "onlret", 6);
#ifdef __OpenBSD__
    flcmp(c_oflag, OLCUC, "olcuc", 5);
#endif
    flcmp(c_oflag, OXTABS, "oxtabs", 6);
    flcmp(c_oflag, ONOEOT, "onoeot", 6);

    flcmp(c_cflag, CREAD, "cread", 5);
    flcmp(c_cflag, CSIZE, "csize", 5);
    flcmp(c_cflag, PARENB, "parenb", 6);
    flcmp(c_cflag, PARODD, "parodd", 6);
    flcmp(c_cflag, HUPCL, "hupcl", 5);
    flcmp(c_cflag, CLOCAL, "clocal", 6);
    flcmp(c_cflag, CSTOPB, "cstopb", 6);
    flcmp(c_cflag, CRTSCTS, "crtscts", 6);
    flcmp(c_cflag, MDMBUF, "mdmbuf", 6);

    if (strncmp((char *) a->c_cc, (char *) b->c_cc, sizeof(a->c_cc)) != 0)
        mention("cc", 2);

    if (a->c_ispeed != b->c_ispeed)
        mention("ispeed", 6);
    if (a->c_ospeed != b->c_ospeed)
        mention("ospeed", 6);

    /* NOTE with terminal borked up may need \r\n depending on what the
     * CR related settings got changed to... */
    if (Col_Offset != 0)
        fputc('\n', stderr);
}

void mention(char *s, int len)
{
    if (Col_Offset == 0) {
        fprintf(stderr, "prte: %s", s);
        Col_Offset += 6 + len;
    } else {
        fprintf(stderr, " %s", s);
        Col_Offset += len + 1;
    }
    if (Col_Offset >= LINELENGTH) {
        Col_Offset = 0;
        fputc('\n', stderr);
    }
}
