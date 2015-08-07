/*
 * Utility for testing the resolver, as host(1) and dig(1) ignored the
 * LOCALDOMAIN environment variable, and writing C code is educational
 * (if somewhat scary to delve down through the old rusty plumbing of
 * the Internet). Probably should also do getaddrinfo(3) to see if that
 * honors LOCALDOMAIN or not.
 *
 * A better option for testing is likely `getent hosts www` than this code.
 *
 * Usage:
 *
 *   resolv www
 *   env LOCALDOMAIN=example.org resolv www
 *
 * Plus a test DNS server that responds as necessary to the above, or
 * use two different existing domains in the resolv.conf search or
 * LOCALDOMAIN environment setting. lib/libc/net/res_init.c on OpenBSD
 * accepts the form "domain search1 search2 ..." so users can apparently
 * set both a domain and a custom search path.
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

// for res_* method
#include <sys/param.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>

#ifdef __DARWIN__
// for C_IN, T_A
#include <arpa/nameser_compat.h>
#include <nameser.h>
#endif

void resolv_eldritch(char *);

int main(int argc, char *argv[])
{
    int ch;

    while ((ch = getopt(argc, argv, "")) != -1) {
        switch (ch) {
        default:
            ;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 1)
        errx(EX_USAGE, "usage: need hostname to lookup as first argument");

    resolv_eldritch(argv[0]);

    exit(EXIT_SUCCESS);
}

/*
 * Ancient, old way of resolving. Something modern would ideally instead
 * use getaddrinfo(3).
 */
void resolv_eldritch(host)
char *host;
{
    int len, status;
    char rhost[MAXHOSTNAMELEN];
    u_char answer[128] = "";
    unsigned char *p;

    len = res_search(host, C_IN, T_A, answer, (int) sizeof(answer));
    if (len == -1)
        errx(EX_NOHOST, "res_search failed: h_errno=%d, hstrerror=%s",
             h_errno, hstrerror(h_errno));

    p = answer;
    p += 12;                    // skip over header [RFC 1035]

    status = dn_expand(answer, answer + len, p, rhost, (int) sizeof(rhost));
    if (status < 0)
        errx(EX_NOHOST, "dn_expand failed");

    printf("%s expands to %s\n", host, rhost);

    /*
     * Next would need to pull the RCODE/ANCOUNT from header to see what
     * happened (instead of +12ing over it, above, find out how many
     * answers there are, parse IPs from answers, etc.
     * sendmail/sm_resolve.* has example code. But just the response
     * hostname resolves whether or not LOCALDOMAIN can tweak
     * unqualified hostnames.
     */
}
