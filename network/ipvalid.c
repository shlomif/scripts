/* ipvalid - validate an IP address via inet_pton(3) and emit via
 * inet_ntop(3) */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);

bool Flag_Quiet = false;

char addr[INET6_ADDRSTRLEN];
struct in_addr v4addr;
struct in6_addr v6addr;

int main(int argc, char *argv[])
{
    int ch, ipret;

#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    while ((ch = getopt(argc, argv, "q")) != -1) {
        switch (ch) {
        case 'q':
            Flag_Quiet = true;
            break;
        case 'h':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc == 0)
        emit_help();

    // may be necessary if IPv6 addresses mis-parsed by inet_ntop(AF_INET,
    //if (!strchr(*argv, ':')) {

    ipret = inet_pton(AF_INET, *argv, &v4addr);
    switch (ipret) {
    case 1:
        if (!Flag_Quiet) {
            if (!inet_ntop(AF_INET, &v4addr, (char *) &addr, INET_ADDRSTRLEN))
                err(EX_OSERR, "inet_ntop() for IPv4 failed");
            puts(addr);
        }
        exit(EXIT_SUCCESS);
    case 0:
        // not parseable, perhaps IPv6?
        break;
    case -1:
        err(EX_OSERR, "inet_pton() for IPv4 failed");
    default:
        errx(EX_OSERR, "unexpected return from inet_pton() for IPv4: %d",
             ipret);
    }

    //} else {

    ipret = inet_pton(AF_INET6, *argv, &v6addr);
    switch (ipret) {
    case 1:
        if (!Flag_Quiet) {
            if (!inet_ntop(AF_INET6, &v6addr, (char *) &addr, INET6_ADDRSTRLEN))
                err(EX_OSERR, "inet_ntop() for IPv6 failed");
            puts(addr);
        }
        exit(EXIT_SUCCESS);
    case 0:
        /* not parseable */
        break;
    case -1:
        err(EX_OSERR, "inet_pton() for IPv6 failed");
    default:
        errx(EX_OSERR, "unexpected return from inet_pton() for IPv6: %d",
             ipret);
    }

    //}

    exit(1);
}

void emit_help(void)
{
    fputs("Usage: ipvalid [-q] ipaddress\n", stderr);
    exit(EX_USAGE);
}
