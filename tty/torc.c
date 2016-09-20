/*
 * Terminal orchestration. Perhaps for something like controlling multiple
 * gdb sessions in lock-step. Uses the somewhat unsavory TIOCSTI ioctl call,
 * so thus requires root or otherwise suitable permissions to operate.
 *
 * Be sure to have some way to exit this program, as it by design passes most
 * everything seen off to the controlled terminals.
 */

#include <sys/ioctl.h>
#include <sys/stat.h>
#if defined(__DARWIN__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <sys/ttycom.h>
#endif

#include <err.h>
#include <fcntl.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <termios.h>
#include <unistd.h>

#define READ_BUF_LEN 32

enum { TORC_TERM_NORM, TORC_TERM_RAW } Terminal_Mode = TORC_TERM_NORM;
struct termios Original_Termios;

void cleanup(void);
void emit_help(void);
void handle_sig(int signo);
void raw_terminal(int fd);
void reset_term(int fd);

int main(int argc, char *argv[])
{
    int ch, *fd_list;
    char *device = NULL;
    char buf[READ_BUF_LEN];
    unsigned int dead_terms, fd_list_len, fdi;
    sigset_t blockthese;
    ssize_t readret, rri;

    while ((ch = getopt(argc, argv, "h?P")) != -1) {
        switch (ch) {
        case 'P':
            warnx("PID %d", getpid());
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

    if (isatty(STDIN_FILENO) == 0)
        err(EX_USAGE, "standard input is not a terminal");

    if ((fd_list = malloc(argc * sizeof(int))) == NULL)
        err(EX_OSERR, "could not malloc() fd list");
    fd_list_len = 0;
    dead_terms = 0;

    while (*argv) {
        if (strchr(*argv, '/') == NULL) {
            if (asprintf(&device, "%s%s", _PATH_DEV, *argv) == -1)
                err(EX_OSERR, "could not asprintf() device path");
        } else {
            device = *argv;
        }
        if ((fd_list[fd_list_len++] = open(device, O_WRONLY)) == -1)
            err(EX_IOERR, "could not open '%s'", device);
        argv++;
    }

    sigemptyset(&blockthese);
    sigaddset(&blockthese, SIGALRM);
    sigaddset(&blockthese, SIGPIPE);
    sigaddset(&blockthese, SIGQUIT);
    sigaddset(&blockthese, SIGTSTP);
    sigaddset(&blockthese, SIGUSR1);
    sigaddset(&blockthese, SIGUSR2);
    sigprocmask(SIG_BLOCK, &blockthese, NULL);
    if (signal(SIGHUP, handle_sig) == SIG_ERR)
        err(EX_OSERR, "could not signal(SIGHUP)");
    if (signal(SIGINT, handle_sig) == SIG_ERR)
        err(EX_OSERR, "could not signal(SIGINT)");
    if (signal(SIGTERM, handle_sig) == SIG_ERR)
        err(EX_OSERR, "could not signal(SIGTERM)");

    atexit(cleanup);
    raw_terminal(STDIN_FILENO);

    while (1) {
        if ((readret = read(STDIN_FILENO, buf, (size_t) READ_BUF_LEN)) == -1) {
            err(EX_IOERR, "read failed");
        }
        if (readret == 0) {
            // EOF on standard input, nothing more to do
            exit(EXIT_SUCCESS);
        } else if (readret < 0) {
            errx(1, "unexpected negative return from read: %ld", readret);
        }
        for (rri = 0; rri < readret; rri++) {
            for (fdi = 0; fdi < fd_list_len; fdi++) {
                if (fd_list[fdi] != -1) {
                    if (ioctl(fd_list[fdi], TIOCSTI, buf + rri) == -1) {
                        close(fd_list[fdi]);
                        fd_list[fdi] = -1;
                        dead_terms++;
                    }
                }
            }
            switch (buf[rri]) {
            case 127:
                // http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
                // cursor backward, erase to right
                write(STDOUT_FILENO, "\x1b[D\x1b[K", 6);
                break;
            case 13:
                /* CR/NL kluge, may not be portable */
                write(STDOUT_FILENO, "\n", 1);
            default:
                write(STDOUT_FILENO, (buf + rri), 1);
            }
        }
        if (dead_terms == fd_list_len) {
            // could false positive, in which case will need to check if
            // ioctl sets errno and then extra logic on that value...
            warnx("notice: no error-free terminals remain");
            exit(EXIT_SUCCESS);
        }
    }

    exit(1);                    /* NOTREACHED */
}

void cleanup(void)
{
    reset_term(STDIN_FILENO);
    write(STDOUT_FILENO, "\n", 1);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: torc dev [dev2 ..]\n");
    exit(EX_USAGE);
}

void handle_sig(int signo)
{
    exit(1);
    // and then the atexit handler should reset the terminal
}

void raw_terminal(int fd)
{
    struct termios terminfo;

    if (tcgetattr(fd, &terminfo) < 0)
        err(EX_OSERR, "could not tcgetattr()");
    Original_Termios = terminfo;

    terminfo.c_cc[VMIN] = 1;
    terminfo.c_cc[VTIME] = 0;

    terminfo.c_cflag &= ~(CSIZE | PARENB);
    terminfo.c_cflag |= (CS8);
    terminfo.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    terminfo.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    terminfo.c_oflag &= ~(OPOST);

    Terminal_Mode = TORC_TERM_RAW;
    if (tcsetattr(fd, TCSAFLUSH, &terminfo) < 0)
        err(EX_OSERR, "could not tcsetattr()");

    // confirm settings
    if (tcgetattr(fd, &terminfo) < 0) {
        tcsetattr(fd, TCSANOW, &Original_Termios);
        err(EX_OSERR, "could not tcgetattr()");
    }
    if (terminfo.c_cc[VMIN] != 1 || terminfo.c_cc[VTIME] != 0 ||
        (terminfo.c_cflag & (CSIZE | PARENB | CS8)) != CS8 ||
        (terminfo.c_iflag & (BRKINT | ICRNL | INPCK | ISTRIP | IXON)) ||
        (terminfo.c_lflag & (ECHO | ICANON | IEXTEN | ISIG)) ||
        (terminfo.c_oflag & (OPOST))) {
        tcsetattr(fd, TCSANOW, &Original_Termios);
        err(EX_OSERR, "did not tcsetattr()");
    }
}

void reset_term(int fd)
{
    if (Terminal_Mode == TORC_TERM_NORM)
        return;
    if (tcsetattr(fd, TCSANOW, &Original_Termios) < 0)
        err(EX_OSERR, "could not reset terminal with tcsetattr()");
    Terminal_Mode = TORC_TERM_NORM;
}
