/*
 * Sender of UDP packets. Mostly practice via a reading of "Beej's Guide
 * to Network Programming" plus a work need to test various UDP related
 * setups. There are probably better ways, altagoobingleduckgo them.
 */

#include "udpfling.h"

int sockfd;
int status;
struct addrinfo hints, *p, *res;

unsigned long counter, prev_sent_count;
uint32_t ncounter;
ssize_t sent_size;
unsigned int backoff = DEFAULT_BACKOFF;

unsigned int time_units, time_units_to_usec;

struct timespec backoff_by, delay_by;
struct timespec when;

struct sigaction act;

int main(int argc, char *argv[])
{
    int arg_offset;
    char *payload;

    Flag_Max_Send = UINT_MAX;   /* how many packets to send (a lot) */

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

    if (Flag_Line_Buf)
        setvbuf(stdout, (char *)NULL, _IOLBF, (size_t) 0);

    time_units = Flag_Nanoseconds ? USEC_IN_SEC : MS_IN_SEC;
    time_units_to_usec = Flag_Nanoseconds ? 1 : USEC_IN_MS;
    delay_by.tv_sec = (long) Flag_Delay / time_units;
    delay_by.tv_nsec = (Flag_Delay % time_units) * time_units_to_usec;

    if ((payload = malloc(Flag_Padding)) == NULL)
        err(EX_OSERR, "could not malloc payload");
    memset(payload, 255, Flag_Padding); /* ones as zeros might compress */

    act.sa_handler = catch_intr;
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) != 0)
        err(EX_OSERR, "could not setup SIGINT handle");

    while (++counter < Flag_Max_Send) {
        ncounter = htonl(counter);
        memcpy(payload, &ncounter, sizeof(ncounter));

        if ((sent_size = sendto(sockfd, payload, Flag_Padding, 0,
                                p->ai_addr, p->ai_addrlen)) < 0) {
            if (errno == ENOBUFS || errno == EINTR) {
                if (!Flag_Flood)
                    warn("retrying sendto (%d):", errno);

                /* But not right away if things going awry */
                backoff_by.tv_sec = backoff / USEC_IN_MS;
                backoff_by.tv_nsec = backoff % USEC_IN_MS;
                nanosleep(&backoff_by, NULL);
                backoff <<= 1;
                if (backoff > MAX_BACKOFF)
                    backoff = MAX_BACKOFF;
            } else
                err(EX_IOERR, "send error");

        } else {
            if ((size_t) sent_size < Flag_Padding)
                errx(EX_IOERR, "sent size less than expected: %ld vs %ld",
                     sent_size, Flag_Padding);

            backoff = DEFAULT_BACKOFF;
        }

        if (counter % Flag_Count == 0) {
            /* also in catch_intr, below */
            if (clock_gettime(CLOCK_REALTIME, &when) == -1)
                err(EX_OSERR, "clock_gettime() failed");
            fprintf(stdout, "%.4Lf %ld\n",
                    when.tv_sec + when.tv_nsec / (long double) NSEC_IN_SEC,
                    counter - prev_sent_count);
            prev_sent_count = counter;
        }
        if (!Flag_Flood)
            nanosleep(&delay_by, NULL);
    }

    close(sockfd);

    exit(EXIT_SUCCESS);
}

void catch_intr(int sig)
{
    if (clock_gettime(CLOCK_REALTIME, &when) == -1)
        err(EX_OSERR, "clock_gettime() failed");
    fprintf(stdout, "%.4Lf %ld\n",
            when.tv_sec + when.tv_nsec / (long double) NSEC_IN_SEC,
            counter - prev_sent_count);
    errx(1, "quit due to signal %d (sent %ld packets)", sig, counter);
}

void emit_usage(void)
{
    errx(EX_USAGE,
         "[-4|-6] [-C maxsend] [-c stati] [-d ms|-f] [-l] [-N] [-P bytes] -p port hostname");
}
