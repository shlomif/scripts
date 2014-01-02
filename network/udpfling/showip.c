/*
 * showip.c -- show IP addresses for a host given on the command line
 * From "Beej's Guide to Network Programming"
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2) {
	fprintf(stderr, "usage: showip hostname\n");
	return 1;
    }
    memset(&hints, 0, sizeof hints);
/* use AF_INET or AF_INET6 to force version */
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
	return 2;
    }
    printf("IP addresses for %s:\n\n", argv[1]);

    for (p = res; p != NULL; p = p->ai_next) {
	void *addr;
	char *ipver;

	/* get the pointer to the address itself, different fields in IPv4
	 * and IPv6 */
	if (p->ai_family == AF_INET) {
	    struct sockaddr_in *ipv4 = (struct sockaddr_in *) p->ai_addr;
	    addr = &(ipv4->sin_addr);
	    asprintf(&ipver, "IPv4");
	} else if (p->ai_family == AF_INET6) {
	    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) p->ai_addr;
	    addr = &(ipv6->sin6_addr);
	    asprintf(&ipver, "IPv6");
	} else {
	    /* unknown family, dunno what to do */
	    abort();
	}

	/* convert the IP to a string and print it */
	inet_ntop(p->ai_family, addr, ipstr, (socklen_t) sizeof(ipstr));
	printf("  %s %s\n", ipver, ipstr);
    }

    freeaddrinfo(res);

    return 0;
}
