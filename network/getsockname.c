/***********************************************************************
 *
 * getsockname(2) practice, as there are fiddly details.
 *
 * Things to read: grep around under /usr/include for the various *.h
 * files that define e.g. 'struct addrinfo' and other important types.
 * Also handy was to create a simple Perl implementation:
 *
 *   #!/usr/bin/env perl
 *   use strict;
 *   use warnings;
 *   use Socket;
 *   
 *   die "Usage: $0 host port\n" if @ARGV != 2;
 *   my ( $host, $port ) = @ARGV;
 *   
 *   socket( my $s, PF_INET, SOCK_DGRAM, 0 ) or die "socket error: $!\n";
 *   connect( $s, pack_sockaddr_in( $port, inet_aton($host) ) )
 *     or die "connect error: $!\n";
 *   
 *   my $mysockaddr = getsockname($s);
 *   my ( $lport, $myaddr ) = sockaddr_in($mysockaddr);
 *   printf( "Connect from [%s]:$lport\n", inet_ntoa($myaddr) );
 *
 * and then to strace(1) that against what my C program was doing for
 * hints as to what I was doing wrong in C.
 *
 * Use nc(1) or similar to create listeners to connect to, if those
 * are lacking.
 */

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <err.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

bool Flag_Stick_Around;

struct addrinfo *remote, hints, *res;
struct sockaddr *localsa;
socklen_t localsa_len;

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, ret, sockfd;
    char ipstr[INET_ADDRSTRLEN];

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    while ((ch = getopt(argc, argv, "46suh?")) != -1) {
        switch (ch) {
        case '4':
            hints.ai_family = AF_INET;
            break;
        case '6':
            hints.ai_family = AF_INET6;
            break;
        case 's':
            Flag_Stick_Around = true;
            break;
        case 'u':
            hints.ai_socktype = SOCK_DGRAM;
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
    if (argc != 2)
        emit_help();

    setvbuf(stdout, (char *)NULL, _IOLBF, (size_t) 0);

    if ((ret = getaddrinfo(argv[0], argv[1], &hints, &res)) != 0)
        errx(EX_NOHOST, "getaddrinfo error: %s", gai_strerror(ret));
    for (remote = res; remote != NULL; remote = remote->ai_next) {
        if ((sockfd =
             socket(remote->ai_family, remote->ai_socktype,
                    remote->ai_protocol)) == -1) {
            warn("socket() error");
            continue;
        }
        /* Do need to connect first, unless you like seeing [::] or
         * 0.0.0.0 as your local address. */
        if (connect(sockfd, remote->ai_addr, remote->ai_addrlen) == -1) {
           warn("connect() error");
           continue;
        }
        break;
    }
    if (remote == NULL)
        errx(EX_IOERR, "could not connect to socket()");

    switch (remote->ai_family) {
    case AF_INET:
        if ((localsa = malloc(sizeof(struct sockaddr_in))) == NULL)
            err(EX_OSERR, "malloc() sockaddr_in failed");
        break;
    case AF_INET6:
        if ((localsa = malloc(sizeof(struct sockaddr_in6))) == NULL)
            err(EX_OSERR, "malloc() sockaddr_in6 failed");
        break;
    default:
        errx(EX_OSERR, "unknown address family???");
    }
    localsa_len = remote->ai_addrlen;

    if (getsockname(sockfd, localsa, &localsa_len) == -1)
        err(EX_OSERR, "getsockname() failed");

    switch (localsa->sa_family) {
    case AF_INET:
        inet_ntop(AF_INET, &(((struct sockaddr_in *) localsa)->sin_addr), ipstr,
                  localsa_len);
        printf("local %s:%u\n", ipstr,
               ntohs(((struct sockaddr_in *) localsa)->sin_port));
        break;
    case AF_INET6:
        inet_ntop(AF_INET6, &(((struct sockaddr_in6 *) localsa)->sin6_addr),
                  ipstr, localsa_len);
        printf("local [%s]:%u\n", ipstr,
               ntohs(((struct sockaddr_in6 *) localsa)->sin6_port));
        break;
    default:
        errx(EX_OSERR, "unknown address family???");
    }

    /* Cheap blocking trick. Then, presumably, `lsof -i -nP` or
     * `netstat` or something would be used to investigate what this
     * program has opened. `fg` will then bring the program back so
     * it can exit. */
    if (Flag_Stick_Around)
        raise(SIGTSTP);

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: getsockname [-4 | -6] [-s] [-u] host port\n");
    exit(EX_USAGE);
}
