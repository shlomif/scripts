/*
 * OpenBSD divert(4) packet mangler. Supports (pseudo)random drops,
 * delays (and if so possibly also duplication), and data payload
 * corruption of packets passed through it. One practical application
 * would be to test how well various protocols or applications cope with
 * the thus mangled packets.
 *
 * TODO - it's mostly sorta working, but...
 *   * IPv6 support has not much been tested, and needs the payload
 *     length calculation figured out for data corruption.
 *   * Ability to delay multiple packets instead of just one. (Plus
 *     how long packets are delayed for instead of automatically
 *     clearing them in the alarm handler.)
 *   * A socket interface (imsg_init(3)?) would be nice so that
 *     the odds could be adjusted on the fly.
 *   * Means to adjust logging or disable that or improve it.
 *   * Daemonize vs. debug support.
 *   * decider() f(x) to be called by both v4 and v6 packet paths
 *     to pick whether DROP,DUP,DELAY,CORRUPT happens. (reduce code dup)
 *
 * In the meantime, fiddle with the args or code as necessary. Needs
 * OpenBSD >=5.5 due to syscalls used. Compile with -DSCRUB_MEMORY
 * to better cleanup (sensitive?) packet contents that may be left
 * otherwise laying around in memory.
 */

#include "diversion.h"

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

float Flag_Corrupt;             // -C
int Flag_Direction;             // -f
float Flag_Drop;                // -D
float Flag_Delay;               // -W
float Flag_Duplicate;           // -R
uint16_t Flag_Divert_Port;      // -p
char *Flag_User;                // -u

volatile sig_atomic_t Sig_Alarm;
volatile sig_atomic_t Sig_Terminate;

struct packet_stash {
    ssize_t nbytes;
    uint8_t *packet;
};
struct packet_stash v4stash, v6stash;

void emit_help(void);
void sig_handle(int sig);

int main(int argc, char *argv[])
{
    int ch;

    ssize_t nbytes;
    uint8_t packet[IP_MAXPACKET];
    ssize_t range;              // how much data payload there is to corrupt

    int fd4;
    socklen_t sin4_len;
    struct ip *ip4_hdr;
    struct sockaddr_in sin4;
    uint8_t hlen, tcphlen;
    struct tcphdr *th;

    int fd6;
    socklen_t sin6_len;
    struct ip6_hdr *ip6_hdr;
    struct sockaddr_in6 sin6;

    int nfds;
    struct pollfd pfd[POLL_FDS];

    struct passwd *pw;

    while ((ch = getopt(argc, argv, "h?C:D:R:W:f:lp:u:")) != -1) {
        switch (ch) {
        case 'C':
            Flag_Corrupt = flagtof(ch, optarg, 0.0, 1.0);
            break;
        case 'D':
            Flag_Drop = flagtof(ch, optarg, 0.0, 1.0);
            break;
        case 'R':
            Flag_Duplicate = flagtof(ch, optarg, 0.0, 1.0);
            break;
        case 'W':
            Flag_Delay = flagtof(ch, optarg, 0.0, 1.0);
            break;
        case 'f':
            Flag_Direction = flagtoul(ch, optarg, 0UL, 2UL);
            if (Flag_Direction)
                Flag_Direction = (Flag_Direction == 1)
                    ? IPPROTO_DIVERT_INIT : IPPROTO_DIVERT_RESP;
            break;
        case 'l':
            setvbuf(stdout, (char *) NULL, _IOLBF, (size_t) 0);
            break;
        case 'p':
            Flag_Divert_Port =
                (uint16_t) flagtoul(ch, optarg, 1UL,
                                    (unsigned long) DIVERT_MAXPORT);
            break;
        case 'u':
            Flag_User = optarg;
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
    if (argc > 0 || Flag_Divert_Port == 0)
        emit_help();

    if (geteuid())
        errx(EX_USAGE, "must be run as root");

    // TODO daemonize vs. debug support
    //daemon(1, 0);

    /* IPv4 setup */
    if ((fd4 = socket(AF_INET, SOCK_RAW, IPPROTO_DIVERT)) == -1)
        err(EX_OSERR, "socket() failed for IPv4");
    if (Flag_Direction) {
        if (setsockopt
            (fd4, IPPROTO_IP, IP_DIVERTFL, &Flag_Direction,
             (socklen_t) sizeof(Flag_Direction)) == -1)
            err(EX_OSERR, "setsockopt() -f failed for IPv4");
    }

    explicit_bzero(&sin4, sizeof(sin4));
    sin4.sin_family = AF_INET;
    sin4.sin_port = htons(Flag_Divert_Port);

    sin4_len = sizeof(struct sockaddr_in);

    if (bind(fd4, (struct sockaddr *) &sin4, sin4_len) == -1)
        err(EX_OSERR, "bind() failed for IPv4");

    v4stash.packet = NULL;

    /* IPv6 setup */
    if ((fd6 = socket(AF_INET6, SOCK_RAW, IPPROTO_DIVERT)) == -1)
        err(EX_OSERR, "socket() failed for IPv6");
    if (Flag_Direction) {
        if (setsockopt
            (fd6, IPPROTO_IP, IP_DIVERTFL, &Flag_Direction,
             (socklen_t) sizeof(Flag_Direction)) == -1)
            err(EX_OSERR, "setsockopt() -f failed for IPv6");
    }

    explicit_bzero(&sin6, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_len = sizeof(struct sockaddr_in6);
    sin6.sin6_port = htons(Flag_Divert_Port);

    sin6_len = sizeof(struct sockaddr_in6);

    if (bind(fd6, (struct sockaddr *) &sin6, sin6.sin6_len) == -1)
        err(EX_OSERR, "bind() failed for IPv6");

    v6stash.packet = NULL;


    if (Flag_User) {
        if ((pw = getpwnam(Flag_User)) == NULL)
            err(EX_OSERR, "getpwnam() for -u user failed");
        if (chroot(pw->pw_dir) == -1)
            err(EX_OSERR, "chroot() failed");
        if (chdir("/") == -1)
            err(EX_OSERR, "chdir(\"/\") failed");
        if (setgroups(1, &pw->pw_gid) ||
            setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
            setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
            errx(EX_OSERR, "could not drop privileges");
    }

    signal(SIGALRM, sig_handle);
    signal(SIGINT, sig_handle);
    signal(SIGTERM, sig_handle);
    ualarm(TICK_MS, TICK_MS);

    explicit_bzero(&pfd, sizeof(pfd));
    pfd[PFD_IPV4].fd = fd4;
    pfd[PFD_IPV4].events = POLLIN;
    pfd[PFD_IPV6].fd = fd6;
    pfd[PFD_IPV6].events = POLLIN;

    for (;;) {
        if ((nfds = poll(pfd, POLL_FDS, -1)) == -1) {
            if (errno != EINTR)
                err(EX_OSERR, "poll() failed");
        }

        if (nfds > 0 && pfd[PFD_IPV4].revents & POLLIN) {
            explicit_bzero(packet, sizeof(packet));
            if ((nbytes =
                 recvfrom(fd4, packet, sizeof(packet), 0,
                          (struct sockaddr *) &sin4, &sin4_len)) == -1) {
                warn("IPv4 recvfrom() error");
                continue;
            }
            if (nbytes < (ssize_t) sizeof(struct ip)) {
                warnx("IPv4 packet is too short");
                continue;
            }

            ip4_hdr = (struct ip *) packet;
            hlen = ip4_hdr->ip_hl << 2;
            if (hlen < sizeof(struct ip) || ntohs(ip4_hdr->ip_len) < hlen
                || nbytes < ntohs(ip4_hdr->ip_len)) {
                warnx("invalid IPv4 packet");
                continue;
            }

            if (arc4random() / (float) UINT32_MAX < Flag_Delay) {
                warnx("delaying a v4 packet...");
                if (!v4stash.packet) {
                    v4stash.nbytes = nbytes;
                    if ((v4stash.packet = malloc((size_t) nbytes)) == NULL) {
                        warn("could not malloc() stash for IPv4 packet");
                    } else {
                        bcopy(packet, v4stash.packet, (size_t) nbytes);
                    }
                }

                if (arc4random() / (float) UINT32_MAX > Flag_Duplicate) {
                    continue;
                }
                warnx("duplicating a v4 packet...");
            } else if (arc4random() / (float) UINT32_MAX < Flag_Drop) {
                continue;
            }

            if (arc4random() / (float) UINT32_MAX < Flag_Corrupt) {
                warnx("corrupting a v4 packet...");
                switch (ip4_hdr->ip_p) {
                case 6:        // TCP
                    th = (struct tcphdr *) (packet + hlen);
                    tcphlen = th->th_off << 2;
                    // TODO or could just discard the packet since the header
                    // is wrong or just plain lying?
                    if (tcphlen < 20) {
                        warnx("IPv4 header len < 20 ??");
                        tcphlen = 20;
                    }
                    hlen += tcphlen;
                    break;
                case 17:       // UDP
                    hlen += 8;
                    break;
                }
                range = nbytes - hlen;

                if (range > 0) {
                    packet[hlen + arc4random_uniform((uint32_t) range)] ^=
                        1 << arc4random_uniform((uint32_t) CHAR_BIT);
                }
            }

            if (sendto
                (fd4, packet, (size_t) nbytes, 0, (struct sockaddr *) &sin4,
                 sin4_len) == -1)
                warn("IPv4 sendto() failed");
        }

        if (nfds > 0 && pfd[PFD_IPV6].revents & POLLIN) {
            explicit_bzero(packet, sizeof(packet));
            if ((nbytes =
                 recvfrom(fd6, packet, sizeof(packet), 0,
                          (struct sockaddr *) &sin6, &sin6_len)) == -1) {
                warn("IPv6 recvfrom() error");
                continue;
            }
            if (nbytes < (ssize_t) sizeof(struct ip6_hdr)) {
                warnx("IPv6 packet is too short");
                continue;
            }
            ip6_hdr = (struct ip6_hdr *) packet;

            if (arc4random() / (float) UINT32_MAX < Flag_Delay) {
                warnx("delaying a v6 packet...");
                if (!v6stash.packet) {
                    v6stash.nbytes = nbytes;
                    if ((v6stash.packet = malloc((size_t) nbytes)) == NULL) {
                        warn("could not malloc() stash for IPv6 packet");
                    } else {
                        bcopy(packet, v6stash.packet, (size_t) nbytes);
                    }
                }

                if (arc4random() / (float) UINT32_MAX > Flag_Duplicate) {
                    continue;
                }
                warnx("duplicating a v6 packet...");
            } else if (arc4random() / (float) UINT32_MAX < Flag_Drop) {
                continue;
            }

            if (arc4random() / (float) UINT32_MAX < Flag_Corrupt) {
                warnx("corrupting a v6 packet... TODO n=%ld plen=%d", nbytes,
                      ip6_hdr->ip6_plen);
                range = 0;

                // NOTE payload len of 0 means jumbo frame?

                if (range > 0) {
                    packet[hlen + arc4random_uniform((uint32_t) range)] ^=
                        1 << arc4random_uniform((uint32_t) CHAR_BIT);
                }
            }

            if (sendto
                (fd6, packet, (size_t) nbytes, 0, (struct sockaddr *) &sin6,
                 sin6_len) == -1)
                warn("IPv6 sendto() failed");
        }

        if (Sig_Terminate) {    // flush any delayed and bail
            if (v4stash.packet) {
                if (sendto
                    (fd4, v4stash.packet, (size_t) v4stash.nbytes, 0,
                     (struct sockaddr *) &sin4, sin4_len) == -1)
                    warn("IPv4 sendto() failed");
#ifdef SCRUB_MEMORY
                explicit_bzero(v4stash.packet, v4stash.nbytes);
#endif
            }
            if (v6stash.packet) {
                if (sendto
                    (fd6, v6stash.packet, (size_t) v6stash.nbytes, 0,
                     (struct sockaddr *) &sin6, sin6_len) == -1)
                    warn("IPv6 sendto() failed");
#ifdef SCRUB_MEMORY
                explicit_bzero(v6stash.packet, v6stash.nbytes);
#endif
            }
#ifdef SCRUB_MEMORY
            explicit_bzero(packet, IP_MAXPACKET);
#endif
            exit(1);
        }
        if (Sig_Alarm) {        // flush any delayed (maybe?)
            if (v4stash.packet) {
                warnx("sending delayed v4 packet...");
                if (sendto
                    (fd4, v4stash.packet, (size_t) v4stash.nbytes, 0,
                     (struct sockaddr *) &sin4, sin4_len) == -1)
                    warn("IPv4 sendto() failed");
#ifdef SCRUB_MEMORY
                explicit_bzero(v4stash.packet, v4stash.nbytes);
#endif
                free(v4stash.packet);
                v4stash.packet = NULL;
            }
            if (v6stash.packet) {
                warnx("sending delayed v6 packet...");
                if (sendto
                    (fd6, v6stash.packet, (size_t) v6stash.nbytes, 0,
                     (struct sockaddr *) &sin6, sin6_len) == -1)
                    warn("IPv6 sendto() failed");
#ifdef SCRUB_MEMORY
                explicit_bzero(v6stash.packet, v6stash.nbytes);
#endif
                free(v6stash.packet);
                v6stash.packet = NULL;
            }
            Sig_Alarm = 0;
        }
    }

    exit(1);                    /* NOTREACHED */
}

void emit_help(void)
{
    fprintf(stderr, "Usage: diversion [...] -p port\n"
            "  -C corrupt odds (0.0 to 1.0)\n"
            "  -D drop odds\n"
            "  -R duplicate odds (if delayed)\n"
            "  -W delay odds\n"
            "  -f [012] direction of match (0=both,1=INIT,2=RESP)\n"
            "  -p divert port (requires pf.conf configuration)\n"
            "  -u user  to drop privs to\n");
    exit(EX_USAGE);
}

void sig_handle(int sig)
{
    switch (sig) {
    case SIGALRM:
        Sig_Alarm = 1;
        break;
    case SIGINT:
    case SIGTERM:
        Sig_Terminate = 1;
        break;
    }
}
