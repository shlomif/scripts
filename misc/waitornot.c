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
pid_t Child_Pid;

void child_signal(int unused);
void emit_help(void);
void reset_term(void);

int main(int argc, char *argv[])
{
    int ch, status;
    char anykey;
    struct termios terminfo;

    while ((ch = getopt(argc, argv, "h?U")) != -1) {
        switch (ch) {
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

    if (!isatty(STDIN_FILENO))
        errx(1, "must have tty to read from");

    if (tcgetattr(STDIN_FILENO, &terminfo) < 0)
        err(EX_OSERR, "could not tcgetattr() on stdin");

    Original_Termios = terminfo;

    // cfmakeraw(3) is a tad too raw and influences output from child;
    // per termios(5) use "Case B" for quick "any" key reads with
    // canonical mode (line-based processing) and echo turned off.
    terminfo.c_cc[VMIN] = 1;
    terminfo.c_cc[VTIME] = 0;
    terminfo.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminfo);
    atexit(reset_term);

    signal(SIGCHLD, child_signal);

    Child_Pid = fork();

    if (Child_Pid == 0) {       // child
        close(STDIN_FILENO);
        signal(SIGCHLD, SIG_DFL);
        status = execvp(*argv, argv);
        warn("could not exec '%s' (%d)", *argv, status);
        _exit(EX_OSERR);

    } else if (Child_Pid > 0) { // parent
        if ((status = read(STDIN_FILENO, &anykey, 1)) < 0)
            err(EX_IOERR, "read() failed??");
        kill(Child_Pid, SIGTERM);

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
    fprintf(stderr, "Usage: waitornot [-U] command [args ..]\n");
    exit(EX_USAGE);
}

void reset_term(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &Original_Termios);
}
