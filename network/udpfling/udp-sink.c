/* udp-sink - listener for UDP packets */

#include "udpfling.h"

int status, sockfd;
socklen_t addr_size;
struct sockaddr_storage client_addr;
/* for setsockopt() SO_REUSEADDR */
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

int any_sock(void);
int mcast_sock(void);
int portnum(char *s);

int main(int argc, char *argv[])
{
    char *payload;
    ssize_t recv_size;

#ifdef __OpenBSD__
    if (pledge("dns inet rpath stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    parse_opts(argc, argv);

    if (Flag_Multicast) {
        sockfd = mcast_sock();
    } else {
        sockfd = any_sock();
    }

    addr_size = sizeof client_addr;

    if ((payload = malloc(Flag_Padding)) == NULL)
        err(EX_OSERR, "could not malloc payload");

    act.sa_handler = catch_intr;
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) != 0)
        err(EX_OSERR, "could not setup SIGINT handle");

    if (Flag_Delay) {
        time_units = Flag_Nanoseconds ? USEC_IN_SEC : MS_IN_SEC;
        time_units_to_usec = Flag_Nanoseconds ? 1 : USEC_IN_MS;
        delay_by.tv_sec = (long) Flag_Delay / time_units;
        delay_by.tv_nsec = (Flag_Delay % time_units) * time_units_to_usec;
    }
    if (Flag_Line_Buf)
        setvbuf(stdout, (char *) NULL, _IOLBF, (size_t) 0);

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
            report_counts();
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

int any_sock(void)
{
    int fd;
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = Flag_AI_Family;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, Flag_Port, &hints, &servinfo)) != 0)
        errx(EX_UNAVAILABLE, "getaddrinfo error: %s", gai_strerror(status));

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            warn("socket error");
            continue;
        }
        if (setsockopt
            (fd, SOL_SOCKET, SO_REUSEADDR, &yes, (socklen_t) sizeof(int))
            == -1) {
            close(fd);
            warn("setsockopt error for SO_REUSEADDR");
            continue;
        }
        if (bind(fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(fd);
            warn("bind error");
            continue;
        }
        break;
    }
    if (p == NULL)
        errx(EX_IOERR, "could not bind to socket");

    freeaddrinfo(servinfo);

    return fd;
}

void catch_intr(int sig)
{
    report_counts();
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}

void emit_usage(void)
{
    fputs("Usage: udp-sink [-4|-6] [-c stati] [-d ms] [-l] [-M addr]"
          " [-N] [-P octets] -p port\n", stderr);
    exit(EX_USAGE);
}

/* TODO also support Source Specific Multicast (SSM) */
int mcast_sock(void)
{
    int fd = -1;
    int ret, port;
    struct in_addr v4addr;
    struct in6_addr v6addr;
    struct ip_mreq m4req;
    struct ipv6_mreq m6req;
    struct sockaddr_in so4addr;
    struct sockaddr_in6 so6addr;
    uint8_t v6any[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    port = portnum(Flag_Port);

    if (Flag_AI_Family == AF_INET6)
        goto DO_SIX;

    if ((ret = inet_pton(AF_INET, Flag_Multicast, &v4addr)) == -1)
        err(1, "inet_pton AF_INET failed");
    if (ret == 1) {
        if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0)
            err(EX_OSERR, "socket failed");
        memset(&m4req, 0, sizeof(m4req));
        m4req.imr_interface.s_addr = htonl(INADDR_ANY);
        m4req.imr_multiaddr.s_addr = v4addr.s_addr;
        if (setsockopt
            (fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &m4req,
             sizeof(m4req)) < 0) {
            err(EX_OSERR, "setsockopt failed to multicast for v4");
        }
        if (setsockopt
            (fd, SOL_SOCKET, SO_REUSEADDR, &yes, (socklen_t) sizeof(int))
            == -1) {
            err(EX_OSERR, "setsockopt(SO_REUSEADDR)");
        }
        memset(&so4addr, 0, sizeof(so4addr));
        so4addr.sin_family = AF_INET;
        so4addr.sin_addr.s_addr = htonl(INADDR_ANY);
        so4addr.sin_port = port;
        if (bind(fd, (struct sockaddr *) &so4addr, sizeof(so4addr)) < 0)
            err(EX_OSERR, "bind v4 multicast failed");
    }
    if (Flag_AI_Family == AF_INET)
        goto DONE;

  DO_SIX:
    if ((ret = inet_pton(AF_INET6, Flag_Multicast, &v6addr)) == -1)
        err(1, "inet_pton AF_INET6 failed");
    if (ret == 1) {
        if ((fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP)) < 0)
            err(EX_OSERR, "socket failed");
        memset(&m6req, 0, sizeof(m6req));
        memcpy(&m6req.ipv6mr_multiaddr, &v6addr,
               sizeof(m6req.ipv6mr_multiaddr));
        if (setsockopt
            (fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *) &m6req,
             sizeof(m6req)) < 0) {
            err(EX_OSERR, "setsockopt failed to multicast for v6");
        }
        if (setsockopt
            (fd, SOL_SOCKET, SO_REUSEADDR, &yes, (socklen_t) sizeof(int))
            == -1) {
            err(EX_OSERR, "setsockopt(SO_REUSEADDR)");
        }
        memset(&so6addr, 0, sizeof(so6addr));
        so6addr.sin6_family = AF_INET6;
        memcpy(so6addr.sin6_addr.s6_addr, v6any, sizeof(v6any));
        so6addr.sin6_port = port;
        if (bind(fd, (struct sockaddr *) &so6addr, sizeof(so6addr)) < 0)
            err(EX_OSERR, "bind v6 multicast failed");
    }

  DONE:
    if (fd == -1)
        err(1, "could not setup multicast address");
    return fd;
}

int portnum(char *s)
{
    struct servent *sent;
    int offset;
    unsigned int pnum;
    if (sscanf(s, "%u%n", &pnum, &offset) == 1) {
        if (*(s + offset) != '\0')
            errx(1, "trailing junk after port number");
        if (pnum < 1 || pnum > 0xFFFF)
            errx(1, "port number out of range");
        return htons(pnum);
    } else {
        if ((sent = getservbyname(s, "udp")) == NULL)
            errx(1, "getservbyname failed");
        return sent->s_port;
    }
}

inline void report_counts(void)
{
    if (clock_gettime(CLOCK_REALTIME, &when) == -1)
        err(EX_OSERR, "clock_gettime() failed");
    fprintf(stdout, "%.4Lf %ld %ld %.2g%% %ld\n",
            when.tv_sec + when.tv_nsec / (long double) NSEC_IN_SEC,
            loss, count_from_client - prev_client_bucket,
            (double) loss / (count_from_client -
                             prev_client_bucket) * 100, count_backtracks);
}
