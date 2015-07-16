/*
 * Utility to pester TCP ports with: connect flood testing. Tested on OpenBSD,
 * Mac OS X. 389-ds had a production flaw that this utility was handy to
 * test whether they had fixed it yet:
 *
 * https://bugzilla.redhat.com/show_bug.cgi?id=668619
 *
 * Previously I was testing with `while true; do nmap -PN -sT ...` in
 * the shell, which caused huge overhead launching the many millions of
 * nmap processes. This vastly more efficient while-loop-around-TCP-connect
 * is much faster, though shifts the bottleneck to the available
 * ephemeral port range of the test system. (Which, on a multi-use system,
 * may adversely affect other programs.) So want something that can auto-tune,
 * or to accept shaping arguments that specify how far the program can go.
 *
 * TODO grab standard tuning algorithm (naive approach I tried had...problems).
 *
 * For greater efficiency, root + raw sockets would be required, as could then
 * drop packets onto the network, and (perhaps) ignore responses?
 *
 * TODO means to throw data out over the connection.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG_PERIOD 60           /* Frequency in seconds of how often to log */

/* TODO rewrite to use sysexits.h */
enum returns { R_OKAY, R_USAGE, R_SYS_ERR };
int exit_status = R_OKAY;

/* Command Line Options */
int nflag;                      /* Don't do name look up */
int vflag;                      /* Verbosity */

int family = AF_UNSPEC;

int main(int argc, char *argv[])
{
    int ch, error, s;
    char *host, *port, *tmp;
    struct addrinfo hints, *res;
    struct timespec rqtp;
    unsigned long req_count, err_count, prev_req_count, prev_err_count;
    long sleep_ms, prev_log_bucket, cur_log_bucket;

    host = NULL;
    port = NULL;
    req_count = 0;
    err_count = 0;
    prev_req_count = 0;
    prev_err_count = 0;
    prev_log_bucket = time(NULL) / LOG_PERIOD;

    // TODO this base rate should be settable by option
    sleep_ms = 10;

    rqtp.tv_sec = 0;
    rqtp.tv_nsec = sleep_ms * 1000 * 1000;

    // TODO option to specify timeout of connection, another to supply
    // count of connections to make (if one wants, say 15 million attempts,
    // so can have program quit after that many), rate of connections...
    // Also option if err_count gets too high to bail?
    while ((ch = getopt(argc, argv, "46nv")) != -1) {
        switch (ch) {
        case '4':
            family = AF_INET;
            break;
        case '6':
            family = AF_INET6;
            break;
        case 'n':
            nflag = 1;
            break;
        case 'v':
            vflag = 1;
            break;
        default:
            ;
        }
    }
    argc -= optind;
    argv += optind;

    host = argv[0];
    if (!host || host[0] == '\0')
        errx(R_USAGE, "no hostname specified");
    port = argv[1];

    /*
     * Support both "host port" and "host:port" calling syntax, though
     * only if IPv4 option forced. IPv6 uses the colon for other things.
     */
    if (family == AF_INET) {
        tmp = strtok(host, ":");
        if (tmp) {
            host = tmp;
            tmp = strtok(NULL, ":");
            if (tmp) {
                if (port)
                    errx(R_USAGE, "cannot mix host:port and host port usages");
                port = tmp;
            }
        }
    }
    if (!port)
        errx(R_USAGE, "no port specified");

    /* play adequately with tee(1) */
    setvbuf(stdout, (char *)NULL, _IOLBF, (size_t) 0);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (nflag)
        hints.ai_flags |= AI_NUMERICHOST;

    // TODO this can return a list; may want option to pick-first or
    // randomize or poke away at all the results, depending.
    if ((error = getaddrinfo(host, port, &hints, &res)))
        errx(R_SYS_ERR, "getaddrinfo: %s", gai_strerror(error));

    while (1) {
        if ((s =
             socket(res->ai_family, res->ai_socktype,
                    res->ai_protocol)) < 0) {
            if (vflag)
                warn("could not create socket");
            err_count++;
        } else {
            /* TODO may need to set other sockopts here (IP_TOS, SO_SNDBUF?) */

            if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
                if (vflag)
                    warn("connect to %s:%s failed", host, port);
                err_count++;
            } else {
                req_count++;
            }
            close(s);
        }

        nanosleep(&rqtp, NULL);

        cur_log_bucket = time(NULL) / LOG_PERIOD;
        if (cur_log_bucket > prev_log_bucket) {
            printf("%ld %ld %ld\n", time(NULL),
                   req_count - prev_req_count, err_count);

            prev_log_bucket = cur_log_bucket;
            prev_req_count = req_count;
            prev_err_count = err_count;

        } else if (cur_log_bucket < prev_log_bucket) {
            /*
             * Will only happen if system time retreats, e.g. via a
             * forced change via ntpdate(8). This throws off stats, so
             * bail out. Probably not worth worrying about, or the
             * stats could perhaps instead include ? that gnuplot or
             * whatever would then ignore.
             */
            errx(R_SYS_ERR, "epoch time went backwards %ld to %ld",
                 prev_log_bucket * LOG_PERIOD, cur_log_bucket * LOG_PERIOD);
        }
    }

    exit(exit_status);
}
