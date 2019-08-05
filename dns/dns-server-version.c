/* dns-server-version - look up DNS server version */

#include <err.h>
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

#ifdef __OpenBSD__
    if (pledge("exec stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    if (argc != 2)
        emit_help();
    if (argv[1] == NULL || *argv[1] == '\0')
        emit_help();

    len = strnlen(argv[1], DNS_MAX_LEN);
    memcpy(&buf[1], argv[1], len);
    buf[0] = '@';
    execlp("dig", "dig", "+short", "-c", "chaos", "-t", "txt", buf,
           "version.bind", (char *) 0);
    err(1, "exec failed");
}

void emit_help(void)
{
    fputs("Usage: dns-server-version hostname-or-ip\n", stderr);
    exit(64);
}
