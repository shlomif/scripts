/*
 * Listener (sink) for UDP packets. Mostly practice via a reading of
 * "Beej's Guide to Network Programming" plus a work need to test
 * various UDP related setups. There are probably better ways,
 * altagoobingleduckgo them.
 */

#include "udpfling.h"

int status, sockfd;
socklen_t addr_size;
struct addrinfo hints, *servinfo, *p;
struct sockaddr_storage client_addr;
/* for setsockopt() SO_REUSEADDR, below */
int yes = 1;

uint32_t ncount_from_client;
unsigned long count_from_client, count_backtracks, prev_client_bucket,
    prev_count_from_client;
long count_from_client_delta;
unsigned long loss;
unsigned long our_count = 1;

struct timespec delay_by;
struct timespec when;

unsigned int time_units, time_units_to_usec;

struct sigaction act;

int main(int argc, char *argv[])
{
    char *payload;
    ssize_t recv_size;

    parse_opts(argc, argv);

    hints.ai_family = Flag_AI_Family;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, Flag_Port, &hints, &servinfo)) != 0) {
        err(EX_UNAVAILABLE, "getaddrinfo error: %s", gai_strerror(status));
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd =
             socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            warn("socket error");
            continue;
        }
        if (setsockopt
            (sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, (socklen_t) sizeof(int))
            == -1) {
            close(sockfd);
            warn("setsockopt error");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            warn("bind error");
            continue;
        }
        break;
    }
    if (p == NULL)
        errx(EX_IOERR, "could not bind to socket");

    freeaddrinfo(servinfo);
    addr_size = sizeof client_addr;

    if (Flag_Line_Buf)
        setvbuf(stdout, (char *)NULL, _IOLBF, (size_t) 0);

    if (Flag_Delay) {
        time_units = Flag_Nanoseconds ? USEC_IN_SEC : MS_IN_SEC;
        time_units_to_usec = Flag_Nanoseconds ? 1 : USEC_IN_MS;
        delay_by.tv_sec = (long) Flag_Delay / time_units;
        delay_by.tv_nsec = (Flag_Delay % time_units) * time_units_to_usec;
    }
    if ((payload = malloc(Flag_Padding)) == NULL)
        err(EX_OSERR, "could not malloc payload");

    act.sa_handler = catch_intr;
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) != 0)
        err(EX_OSERR, "could not setup SIGINT handle");

    while (1) {
        if ((recv_size =
             recvfrom(sockfd, payload, Flag_Padding, 0,
                      (struct sockaddr *) &client_addr, &addr_size)) < 0)
            err(EX_IOERR, "recv error");
        if (recv_size == 0)
            errx(EX_IOERR, "connection closed by peer");
        if ((size_t) recv_size < Flag_Padding)
            errx(EX_IOERR, "recv size less than expected");
        if ((size_t) recv_size > Flag_Padding)
            errx(EX_IOERR, "recv size greater than expected %ld vs %ld",
                 recv_size, Flag_Padding);

        memcpy(&ncount_from_client, payload, sizeof(ncount_from_client));
        count_from_client = ntohl(ncount_from_client);
        count_from_client_delta =
            (long) count_from_client - (long) prev_count_from_client;

        if (count_from_client_delta < 0) {
            warnx("negative delta at %ld", count_from_client);
            count_backtracks++;
        } else if (count_from_client_delta != 1)
            loss += (unsigned long) count_from_client_delta;

        if (our_count++ % Flag_Count == 0) {
            /* also in catch_intr, below */
            if (clock_gettime(CLOCK_REALTIME, &when) == -1)
                err(EX_OSERR, "clock_gettime() failed");
            fprintf(stdout, "%.4Lf %ld %ld %.2g%% %ld\n",
                    when.tv_sec + when.tv_nsec / (long double) NSEC_IN_SEC,
                    loss, count_from_client - prev_client_bucket,
                    (double) loss / (count_from_client -
                                     prev_client_bucket) * 100,
                    count_backtracks);
            count_backtracks = 0;
            loss = 0;
            prev_client_bucket = count_from_client;
        }
        if (Flag_Delay)
            nanosleep(&delay_by, NULL);

        prev_count_from_client = count_from_client;
    }

    exit(EXIT_SUCCESS);
}

void catch_intr(int sig)
{
    if (clock_gettime(CLOCK_REALTIME, &when) == -1)
        err(EX_OSERR, "clock_gettime() failed");
    fprintf(stdout, "%.4Lf %ld %ld %.2f%% %ld\n",
            when.tv_sec + when.tv_nsec / (long double) NSEC_IN_SEC,
            loss, count_from_client - prev_client_bucket,
            (float) loss / (count_from_client - prev_client_bucket) * 100,
            count_backtracks);
    errx(1, "quit due to signal %d (recv %ld packets)", sig, our_count);
}

void emit_usage(void)
{
    errx(EX_USAGE, "[-4|-6] [-c stati] [-d ms] [-l] [-N] [-P bytes] -p port");
}
