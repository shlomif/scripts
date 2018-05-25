/* portpester - pesters a TCP port with lots of connections */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <err.h>
#ifdef SYSDIG
#include <fcntl.h>
#endif
#include <getopt.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#define MS_TO_NANOSEC 1000000U
#define MS_TO_MICROSEC 1000U

int Flag_BaseDelay = 10;        /* b (milliseconds) */
int Flag_LogPeriod = 1;         /* L (seconds) */
int Flag_NumericHost;           /* n - AI_NUMERICHOST */
int Flag_Timeout = 10;          /* t (milliseconds) */

#ifdef SYSDIG
int nullfd;
#endif

void emit_help(void);
void handle_alarm(int sig);
void pester(struct addrinfo *target);

int main(int argc, char *argv[])
{
    char *host = NULL;
    char *port = NULL;
    char *tmp;
    int ch;
    int family = AF_UNSPEC;
    struct addrinfo hints, *target;

    while ((ch = getopt(argc, argv, "h?46b:L:nt:")) != -1) {
        switch (ch) {
        case '4':
            family = AF_INET;
            break;
        case '6':
            family = AF_INET6;
            break;
        case 'b':
            Flag_BaseDelay = (int) flagtoul(ch, optarg, 1UL, 1000UL);
            break;
        case 'L':
            Flag_LogPeriod = (int) flagtoul(ch, optarg, 1UL, 86400UL);
            break;
        case 'n':
            Flag_NumericHost = AI_NUMERICHOST;
            break;
        case 't':
            Flag_Timeout = (int) flagtoul(ch, optarg, 1UL, 1000UL);
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

    host = argv[0];
    if (!host || host[0] == '\0')
        emit_help();
    port = argv[1];

    /* support both "host port" and "host:port" calling syntax, though
     * only if IPv4 option forced. v6 uses the colon for other things */
    if (family == AF_INET) {
        tmp = strtok(host, ":");
        if (tmp) {
            host = tmp;
            tmp = strtok(NULL, ":");
            if (tmp) {
                if (port)
                    errx(1, "cannot mix host:port with host port form");
                port = tmp;
            }
        }
    }
    if (!port || port[0] == '\0')
        emit_help();

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = family;
    hints.ai_flags |= Flag_NumericHost;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;

    if ((ch = getaddrinfo(host, port, &hints, &target)))
        errx(1, "getaddrinfo: %s", gai_strerror(ch));

    setvbuf(stdout, (char *) NULL, _IOLBF, (size_t) 0);

#ifdef SYSDIG
    if ((nullfd = open("/dev/null", O_WRONLY)) == -1)
        err(EX_IOERR, "could not open /dev/null");
#endif

    pester(target);

    /* NOTREACHED */
    exit(1);
}

void emit_help(void)
{
    fprintf(stderr,
            "Usage: portpester [-4|-6] [-b delay] [-L period] [-n] [-t timeout] host port\n");
    exit(EX_USAGE);
}

void handle_alarm(int sig)
{
    ;
}

void pester(struct addrinfo *target)
{
    int s;
    long cur_log_bucket, prev_log_bucket;
    struct itimerval cancel, timeout;
    struct timespec delay;
    time_t epoch, prev_epoch;
    unsigned long err_count, req_count;

    memset(&cancel, 0, sizeof(struct itimerval));
    memset(&timeout, 0, sizeof(struct itimerval));
    timeout.it_value.tv_usec = Flag_Timeout * MS_TO_MICROSEC;

    delay.tv_sec = 0;
    delay.tv_nsec = Flag_BaseDelay * MS_TO_NANOSEC;

    req_count = 0;
    err_count = 0;
    prev_log_bucket = time(NULL) / Flag_LogPeriod;

    if (signal(SIGALRM, handle_alarm) == SIG_ERR)
        err(EX_SOFTWARE, "could not handle ALRM signal()");

    if (siginterrupt(SIGALRM, 1) == -1)
        err(EX_SOFTWARE, "could not siginterrupt()");

    prev_epoch = time(NULL);
    prev_log_bucket = prev_epoch / Flag_LogPeriod;

    while (1) {
        setitimer(ITIMER_REAL, &timeout, NULL);
#ifdef SYSDIG
        dprintf(nullfd, ">::pp-req::\n");
#endif
        if ((s =
             socket(target->ai_family, target->ai_socktype,
                    target->ai_protocol)) < 0) {
            err_count++;
        } else {
            if (connect(s, target->ai_addr, target->ai_addrlen) < 0) {
                err_count++;
            } else {
                req_count++;
            }
            close(s);
        }
        setitimer(ITIMER_REAL, &cancel, NULL);
#ifdef SYSDIG
        dprintf(nullfd, "<::pp-req::\n");
#endif
        nanosleep(&delay, NULL);

        epoch = time(NULL);
        cur_log_bucket = epoch / Flag_LogPeriod;
        if (cur_log_bucket > prev_log_bucket) {
            prev_log_bucket = cur_log_bucket;
            printf("%ld %ld %ld\n", prev_epoch, req_count, err_count);
            err_count = 0;
            req_count = 0;
            prev_epoch = epoch;
        }
    }
}
