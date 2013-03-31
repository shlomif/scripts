/*
 * Sender of UDP packets. Mostly practice via a reading of "Beej's Guide
 * to Network Programming" plus a work need to test various UDP related
 * setups. There are probably better ways, altagoobingleduckgo them.
 */

#include "udpfling.h"

int sockfd;
int status;
struct addrinfo hints, *p, *res;

int main(int argc, char *argv[])
{
    extern int Flag_AI_Family;
    extern int Flag_Count;
    extern long Flag_Delay;
    extern int Flag_Flood;
    extern char Flag_Port[MAX_PORTNAM_LEN];

    int arg_offset;

    struct timespec rqtp;

    ssize_t sent_size;
    unsigned long counter = 1;
    uint32_t ncounter;

    Flag_Count = INT_MAX;       /* how many packets to send */

    arg_offset = parse_opts(argc, argv);
    argc -= arg_offset;
    argv += arg_offset;

    if (argc == 0 || argv[0] == NULL)
        emit_usage();

    hints.ai_family = Flag_AI_Family;
    hints.ai_socktype = SOCK_DGRAM;

    if ((status = getaddrinfo(argv[0], Flag_Port, &hints, &res)) != 0) {
        errx(EX_UNAVAILABLE, "getaddrinfo error: %s", gai_strerror(status));
    }

    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd =
             socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            warn("socket error");
            continue;
        }

        break;
    }
    if (p == NULL)
        errx(EX_IOERR, "could not bind to socket");

    rqtp.tv_sec = Flag_Delay / MS_IN_SEC;
    rqtp.tv_nsec = (Flag_Delay % MS_IN_SEC) * 1000 * 1000;

    while (counter < Flag_Count) {
        ncounter = htonl(counter++);
        /* TODO also need a packet-pad-to-size-X Flag
         * (listener/sink would need to be aware of that when parsing count) */
        if ((sent_size = sendto(sockfd, &ncounter, sizeof(ncounter), 0,
                                p->ai_addr, p->ai_addrlen)) < 0)
            err(EX_IOERR, "send error");
        if (sent_size < sizeof(ncounter))
            errx(EX_IOERR, "sent size less than expected");

        if (!Flag_Flood)
            nanosleep(&rqtp, NULL);
    }

    close(sockfd);

    exit(EXIT_SUCCESS);
}

void emit_usage(void)
{
    errx(EX_USAGE, "[-4|-6] [-c n] [-d ms|-f] -p port hostname");
}
