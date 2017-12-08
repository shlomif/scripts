/* A buggy service daemon. Presumably for testing. Also, libevent practice.
 *
 *
 * Options:
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
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <syslog.h>
#include <unistd.h>

// libevent2 - need at least 2.0.19 for EVENT_LOG_* macro names
#include <event2/event.h>

#ifdef SYSTEMD
#include <systemd/sd-daemon.h>
#define SYSTEMD_UNSET_ENV 1
#endif

int Flag_Cleanup;               // -C
const char *Flag_FifoFile;      // -f file
int Flag_Foreground;            // -I
const char *Flag_PidFile;       // -p file

int Fifo_FD = -1;

void blow_up(int unused);
void clean_out(void);
void do_exit(int unused);
void emit_help(void);
void ev_log2syslog(int severity, const char *msg);
void fifo_read(evutil_socket_t fd, short event, void *unused);

int main(int argc, char *argv[])
{
    int ch, fd;
    struct event *ev_fifo;
    struct event_base *ev_base;

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
    // NOTE must use syslog past here as may be daemonized
    openlog("buggyservd", LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_DAEMON);

    signal(SIGUSR1, blow_up);
    signal(SIGUSR2, do_exit);

    if (Flag_PidFile) {
        if ((fd = open(Flag_PidFile, O_CREAT | O_EXCL | O_WRONLY, 0666)) < 0) {
            syslog(LOG_ERR, "could not open pid file '%s': %s", Flag_PidFile,
                   strerror(errno));
            exit(1);
        }
        dprintf(fd, "%d\n", getpid());
        close(fd);
    }

    event_set_log_callback(ev_log2syslog);
    if ((ev_base = event_base_new()) == NULL) {
        syslog(LOG_ERR, "event_base_new() failed");
        exit(1);
    }

    if (Flag_FifoFile) {
        if (mkfifo(Flag_FifoFile, 0666) != 0) {
            syslog(LOG_ERR, "could not mkfifo '%s': %s", Flag_FifoFile,
                   strerror(errno));
            exit(1);
        }
        if ((Fifo_FD = open(Flag_FifoFile, O_NONBLOCK | O_RDWR)) < 0) {
            syslog(LOG_ERR, "could not open fifo '%s': %s", Flag_FifoFile,
                   strerror(errno));
            exit(1);
        }

        ev_fifo =
            event_new(ev_base, Fifo_FD, EV_READ | EV_PERSIST, fifo_read, NULL);
        event_add(ev_fifo, NULL);
    }

    if (Flag_Cleanup)
        atexit(clean_out);

#ifdef SYSTEMD
    sd_notify(SYSTEMD_UNSET_ENV, "READY=1\n");
#endif

    syslog(LOG_NOTICE, "up and running");

    while (1) {
        event_base_dispatch(ev_base);
        // so that a event_base_loopexit() can be used to exit cleanly
        if (event_base_got_exit(ev_base))
            break;
    }
    exit(EXIT_SUCCESS);
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
    // which then in theory should call the atexit-registered function
    // clean_out() if necessary
    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: buggyservd [-C] [-f fifofile] [-I] [-p pidfile]\n");
    exit(EX_USAGE);
}

void ev_log2syslog(int severity, const char *msg)
{
    int priority;
    switch (severity) {
    case EVENT_LOG_ERR:
        priority = LOG_ERR;
        break;
    case EVENT_LOG_WARN:
        priority = LOG_WARNING;
        break;
    case EVENT_LOG_MSG:
        priority = LOG_INFO;
        break;
    default:
        priority = LOG_DEBUG;
    }
    syslog(priority, "libevent: %s", msg);
}

void fifo_read(evutil_socket_t fd, short event, void *unused)
{
    char buf[96];
    int len;
    len = read(fd, buf, sizeof(buf) - 1);
    // for informational purposes; might act on input if need more
    // commands than what can do via signals
    if (len > 0) {
        buf[len] = '\0';
        syslog(LOG_NOTICE, "fifo input: %s", buf);
    }
}
