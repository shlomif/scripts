/* dns-server-version - look up DNS server version */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define DNS_MAX_LEN 253

char buf[DNS_MAX_LEN + 2];

void emit_help(void);

int main(int argc, char *argv[])
{
    size_t len;
    if (argc != 2)
        emit_help();
    argv++;
    if (argv[0] == NULL || argv[0][0] == '\0')
        emit_help();
    len = strnlen(*argv, DNS_MAX_LEN);
    memcpy(&buf[1], *argv, len);
    buf[0] = '@';
    execlp("dig", "dig", "+short", "-c", "chaos", "-t", "txt", buf,
           "version.bind", (char *) 0);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: dns-server-version hostname-or-ip\n");
    exit(64);
}
