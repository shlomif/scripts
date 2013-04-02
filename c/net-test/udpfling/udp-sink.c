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
int yes = 1;                    // for setsockopt() SO_REUSEADDR, below

uint32_t ncount_from_client;
unsigned long count_from_client;
unsigned long prev_client_bucket;
unsigned long prev_count_from_client;
long count_from_client_delta;
unsigned long loss;
unsigned long our_count = 1;

struct timespec delay_by;
struct timeval when;

int main(int argc, char *argv[])
{
    extern int Flag_AI_Family;
    extern unsigned int Flag_Count;
    extern unsigned int Flag_Delay;
    extern unsigned int Flag_Padding;
    extern char Flag_Port[MAX_PORTNAM_LEN];

    Flag_Count = 10000;         /* how often to print stats */

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
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
            -1) {
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

    setsid();
    fclose(stdin);

    if (Flag_Line_Buf)
        setlinebuf(stdout);

    if (Flag_Delay) {
        delay_by.tv_sec = Flag_Delay / MS_IN_SEC;
        delay_by.tv_nsec = (Flag_Delay % MS_IN_SEC) * 1000 * 1000;
    }

    if ((payload = malloc(Flag_Padding)) == NULL)
        err(EX_OSERR, "could not malloc payload");

    while (1) {
        if ((recv_size =
             recvfrom(sockfd, payload, Flag_Padding, 0,
                      (struct sockaddr *) &client_addr, &addr_size)) < 0)
            err(EX_IOERR, "recv error");
        if (recv_size == 0)
            errx(EX_IOERR, "connection closed by peer");
        if (recv_size < Flag_Padding)
            errx(EX_IOERR, "recv size less than expected");
        if (recv_size > Flag_Padding)
            errx(EX_IOERR, "recv size greater than expected %ld vs %d",
                 (unsigned long) recv_size, Flag_Padding);

        memcpy(&ncount_from_client, payload, sizeof(ncount_from_client));
        count_from_client = ntohl(ncount_from_client);
        count_from_client_delta = count_from_client - prev_count_from_client;

        if (count_from_client_delta < 0)
            warnx("negative delta at %ld", count_from_client);
        else if (count_from_client_delta != 1)
            loss += count_from_client_delta;

        if (our_count++ % Flag_Count == 0) {
            gettimeofday(&when, NULL);
            fprintf(stdout, "%.4f %ld %ld %.2f%%\n",
                    when.tv_sec + (double) when.tv_usec / USEC_IN_SEC,
                    loss, count_from_client - prev_client_bucket,
                    (float) loss / (count_from_client -
                                    prev_client_bucket) * 100);
            loss = 0;
            our_count = 1;
            prev_client_bucket = count_from_client;
        }

        if (Flag_Delay)
            nanosleep(&delay_by, NULL);

        prev_count_from_client = count_from_client;
    }

    exit(EXIT_SUCCESS);
}

void emit_usage(void)
{
    errx(EX_USAGE, "[-4|-6] [-c n] [-d ms] [-l] [-P bytes] -p port");
}
