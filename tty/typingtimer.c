/* typingtimer - at what rate does someone (or something) type at? */

#include <sys/time.h>

#include <err.h>
#include <ncurses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#define NSEC_IN_SEC 1000000000

FILE *log_fh;

void cleanup(void);
void handle_sig(int unused);

int main(int argc, char *argv[])
{
    int ch, row, col;
    struct timespec before, after;

#ifdef __OpenBSD__
    if (pledge("cpath rpath stdio tty wpath unveil", NULL) == -1)
        err(1, "pledge failed");
    if (unveil("/", "r") == -1)
        err(1, "unveil failed");
#endif

    if (argc != 2) {
        fputs("dbg Usage: typingtimer logfile\n", stderr);
        exit(EX_USAGE);
    }

#ifdef __OpenBSD__
    if (unveil(argv[1], "crw") == -1)
        err(1, "unveil failed");
    if (unveil(NULL, NULL) == -1)
        err(1, "unveil failed");
#endif

    if ((log_fh = fopen(argv[1], "w+")) == NULL)
        err(EX_IOERR, "could not write '%s'", argv[1]);

    initscr();
    atexit(cleanup);
    signal(SIGINT, handle_sig);
    signal(SIGPIPE, handle_sig);
    signal(SIGTERM, handle_sig);
    signal(SIGUSR1, handle_sig);
    raw();
    noecho();
    cbreak();
    clearok(stdscr, 1);

#ifdef __OpenBSD__
    if (pledge("stdio tty", NULL) == -1)
        err(1, "pledge failed");
#endif

    while (1) {
        clock_gettime(CLOCK_REALTIME, &before);
        ch = getch();
        clock_gettime(CLOCK_REALTIME, &after);

        fprintf(log_fh, "%Lf\n",
                (after.tv_sec - before.tv_sec) + (after.tv_nsec -
                                                  before.tv_nsec) /
                (long double) NSEC_IN_SEC);

        if (ch == erasechar()) {
            getyx(stdscr, row, col);
            if (col > 0) {
                mvaddch(row, col - 1, ' ');
                move(row, col - 1);
            }
        } else if (ch == killchar()) {
            getyx(stdscr, row, col);
            move(row, 0);
            clrtoeol();
        } else if (ch == '\033') {
            break;
        } else {
            echochar(ch);
        }
    }

    exit(EXIT_SUCCESS);
}

void handle_sig(int unused)
{
    fclose(log_fh);
    endwin();
    exit(EXIT_SUCCESS);
}

void cleanup(void)
{
    fclose(log_fh);
    endwin();
}
