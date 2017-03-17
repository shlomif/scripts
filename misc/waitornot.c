/* waits for a command to complete, or not (on user input) */

#ifdef __linux__
#define _POSIX_SOURCE
#include <sys/types.h>
#endif

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <termios.h>
#include <unistd.h>

int Flag_UserOkay;              // -U

struct termios Original_Termios;

void child_signal(int unused);
void emit_help(void);
void reset_term(void);

int main(int argc, char *argv[])
{
    int ch, status;
    char anykey;
    pid_t child_pid;
    struct termios terminfo;
    tcflag_t lflags_nay = ICANON | ECHO;

    while ((ch = getopt(argc, argv, "h?IU")) != -1) {
        switch (ch) {
        case 'I':
            lflags_nay |= ISIG;
            break;
        case 'U':
            Flag_UserOkay = 1;
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

    if (argc == 0)
        emit_help();

    if (!isatty(STDIN_FILENO))
        errx(1, "must have tty to read from");

    if (tcgetattr(STDIN_FILENO, &terminfo) < 0)
        err(EX_OSERR, "could not tcgetattr() on stdin");

    Original_Termios = terminfo;

    /* cfmakeraw(3) is a tad too raw and influences output from child;
     * per termios(5) use "Case B" for quick "any" key reads with
     * canonical mode (line-based processing), echo (to hide the key the
     * user mashes), and ^Z disabled. ISIG on, unless it is not.
     */
    terminfo.c_cc[VMIN] = 1;
    terminfo.c_cc[VTIME] = 0;
    terminfo.c_lflag |= ISIG;
    terminfo.c_lflag &= ~lflags_nay;
    terminfo.c_cc[VSUSP] = _POSIX_VDISABLE;
// PORTABILITY not everyone (notably not Linux) has this extra
// complication of "delayed suspend" via ^Y; this really should be
// detected by autoconf or the like. APUE 2nd edition indicates that
// Solaris also supports this construct.
#if defined(__APPLE__) || defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__sun)
    terminfo.c_cc[VDSUSP] = _POSIX_VDISABLE;
#endif

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminfo);
    atexit(reset_term);

    signal(SIGCHLD, child_signal);

    child_pid = fork();

    if (child_pid == 0) {       // child
        freopen("/dev/null", "r", stdin);
        signal(SIGCHLD, SIG_DFL);
        status = execvp(*argv, argv);
        warn("could not exec '%s' (%d)", *argv, status);
        _exit(EX_OSERR);

    } else if (child_pid > 0) { // parent
        if ((status = read(STDIN_FILENO, &anykey, 1)) < 0)
            err(EX_IOERR, "read() failed??");
        kill(child_pid, SIGTERM);

    } else {
        err(EX_OSERR, "could not fork");
    }

    exit(Flag_UserOkay ? 0 : 1);
}

void child_signal(int unused)
{
    // might try to pass along the exit status of the child, but that's
    // extra work and complication...
    exit(0);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: waitornot [-I] [-U] command [args ..]\n");
    exit(EX_USAGE);
}

void reset_term(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &Original_Termios);
}
