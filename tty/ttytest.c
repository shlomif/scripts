/* Iterates characters to see what happens to a terminal via the TIOCSTI
 * ioctl. What state the terminal is in and what it is running will also
 * be important, as for example zsh may be configured to react to
 * certain characters, there may or may not be signal handlers, etc. */

#include <sys/ioctl.h>
#include <sys/stat.h>
#if defined(__FreeBSD__) || defined(__OpenBSD__)
#include <sys/ttycom.h>
#endif

#include <err.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <termios.h>
#include <unistd.h>

#define MS_TO_MICROSEC 1000U

struct termios old, new;

void cleanup(int sig);

int main(int argc, char *argv[])
{
    int ch, fd;
    if (argc != 2 || (fd = open(argv[1], O_WRONLY)) == -1)
        err(EX_IOERR, "could not open device '%s'", argv[1]);
    tcgetattr(STDIN_FILENO, &old);
    signal(SIGHUP, cleanup);
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);
    signal(SIGPIPE, cleanup);
    signal(SIGALRM, cleanup);
    signal(SIGUSR1, cleanup);
    signal(SIGUSR2, cleanup);
    signal(SIGTSTP, SIG_IGN);
    new = old;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
    for (unsigned char c = 0;; c++) {
        // EOT may close terminal; this renders subsequent tests difficult
        if (c == 4)
            continue;
        fprintf(stderr, "DBG write %i...\n", c);
        ioctl(fd, TIOCSTI, &c);
        if (c == UINT8_MAX)
            break;
        //usleep(1000 * MS_TO_MICROSEC);
        ch = getchar();
    }
    cleanup(0);
}

void cleanup(int sig)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    exit(sig);
}
