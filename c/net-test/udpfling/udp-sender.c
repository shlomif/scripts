/*
 * Sender of UDP packets. Mostly practice via a reading of "Beej's Guide
 * to Network Programming" plus a work need to test various UDP related
 * setups. There are probably better ways, altagoobingleduckgo them.
 */

#include "udpfling.h"

int sockfd;
int status;
struct addrinfo hints, *p, *res;

unsigned long counter;
uint32_t ncounter;
ssize_t sent_size;

struct timespec delay_by;

int main(int argc, char *argv[])
{
    extern int Flag_AI_Family;
    extern unsigned int Flag_Count;
    extern unsigned int Flag_Delay;
    extern int Flag_Flood;
    extern unsigned int Flag_Padding;
    extern char Flag_Port[MAX_PORTNAM_LEN];

    Flag_Count = UINT_MAX;      /* how many packets to send */

    int arg_offset;
    char *payload;

    arg_offset = parse_opts(argc, argv);
    argc -= arg_offset;
    argv += arg_offset;

    if (argc == 0 || argv[0] == NULL) {
        warnx("no hostname specified");
        emit_usage();
    }

    hints.ai_family = Flag_AI_Family;
    hints.ai_socktype = SOCK_DGRAM;

    if ((status = getaddrinfo(argv[0], Flag_Port, &hints, &res)) != 0) {
        errx(EX_NOHOST, "getaddrinfo error: %s", gai_strerror(status));
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

    setsid();
    fclose(stdin);

    delay_by.tv_sec = Flag_Delay / MS_IN_SEC;
    delay_by.tv_nsec = (Flag_Delay % MS_IN_SEC) * 1000 * 1000;

    if ((payload = malloc(Flag_Padding)) == NULL)
        err(EX_OSERR, "could not malloc payload");
    memset(payload, 255, Flag_Padding); /* ones as zeros might compress */

    while (++counter < Flag_Count) {
        ncounter = htonl(counter);
        memcpy(payload, &ncounter, sizeof(ncounter));

        if ((sent_size = sendto(sockfd, payload, Flag_Padding, 0,
                                p->ai_addr, p->ai_addrlen)) < 0)
            err(EX_IOERR, "send error");
        if (sent_size < Flag_Padding)
            errx(EX_IOERR, "sent size less than expected");

        if (!Flag_Flood)
            nanosleep(&delay_by, NULL);
    }

    close(sockfd);

    exit(EXIT_SUCCESS);
}

void emit_usage(void)
{
    errx(EX_USAGE, "[-4|-6] [-c n] [-d ms|-f] [-P bytes] -p port hostname");
}
