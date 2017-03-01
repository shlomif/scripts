/* A buggy service daemon. Presumably for testing. Also, libevent practice.
 *
 * In particular, this program offers options to run in the foreground
 * (by default daemonizing), segfault on demand (USR1), the ability to
 * create a PID file or FIFO, and (optionally and with a USR2) to
 * perhaps cleanup said files on exit. One can then test out e.g.
 * monit(1) or runit(8) to see how they cope with a specifically badly
 * behaved process.
 *
 * Options:
 *   -C      attempt to cleanup PID and FIFO at exit.
 *   -f fifo create and listen to given filename as FIFO.
 *   -I      do not daemonize; run in the foreground.
 *   -p file create PID file.
 *
 * Signals:
 *   USR1    causes a segfault (or bus error, depending).
 *   USR2    exits the program.
 *
 * With no FIFO given, the process will likely take up 100% of the CPU.
 * This is a feature.
 */

#ifdef __linux__
#define _BSD_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <syslog.h>
#include <unistd.h>

// libevent2 (version 2.1.8 via macports)
#include <event2/event.h>
#include <event2/bufferevent.h>

int Flag_Cleanup;               // -C
const char *Flag_FifoFile;      // -f file
int Flag_Foreground;            // -I
const char *Flag_PidFile;       // -p file

int Fifo_FD = -1;

void blow_up(int unused);
void clean_out(void);
void do_exit(int unused);
void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, fd;
    struct event_base *ev_base;
    struct bufferevent *ev_fifo;

    while ((ch = getopt(argc, argv, "Cf:h?Ip:")) != -1) {
        switch (ch) {
        case 'C':
            Flag_Cleanup = 1;
            break;
        case 'f':
            Flag_FifoFile = optarg;
            break;
        case 'I':
            Flag_Foreground = 1;
            break;
        case 'p':
            Flag_PidFile = optarg;
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

    if (!Flag_Foreground) {
        // NOTE deprecated on Mac OS X 10.11 so may need to do this
        // manually as detailed in perldoc perlipc
        if (daemon(0, 0) != 0)
            err(EX_OSERR, "could not daemonize");
    }

    openlog("buggyservd", LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_DAEMON);

    signal(SIGUSR1, blow_up);
    signal(SIGUSR2, do_exit);

    ev_base = event_base_new();

    if (Flag_FifoFile) {
        if (mkfifo(Flag_FifoFile, 0666) != 0)
            syslog(LOG_ERR, "could not mkfifo '%s': %m", Flag_FifoFile);
        if ((Fifo_FD = open(Flag_FifoFile, O_NONBLOCK | O_RDONLY)) < 0)
            syslog(LOG_ERR, "could not open fifo '%s': %m", Flag_FifoFile);
        ev_fifo = bufferevent_socket_new(ev_base, Fifo_FD, 0);
        bufferevent_enable(ev_fifo, EV_READ | EV_WRITE);
    }

    if (Flag_PidFile) {
        if ((fd = open(Flag_PidFile, O_CREAT | O_EXCL | O_WRONLY, 0666)) < 0)
            syslog(LOG_ERR, "could not open pid file '%s': %m", Flag_PidFile);
        dprintf(fd, "%d\n", getpid());
        close(fd);
    }

    if (Flag_Cleanup)
        atexit(clean_out);

    while (1) {
        event_base_loop(ev_base, 0);
    }
    exit(1);
}

void blow_up(int unused)
{
    char *message = "rad days";
    *message = 's';
}

void clean_out(void)
{
    if (Fifo_FD != -1) {
        close(Fifo_FD);
        unlink(Flag_FifoFile);
    }
    if (Flag_PidFile)
        unlink(Flag_PidFile);
}

void do_exit(int unused)
{
    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: buggyservd [-C] [-f fifofile] [-I] [-p pidfile]\n");
    exit(EX_USAGE);
}
