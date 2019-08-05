/* whatargs - similar to `echo ... | hexdump -C` */

#include <err.h>
#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);

int main(int argc, char *argv[])
{
    char *c;

#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    setlocale(LC_ALL, "C");

    argv++;
    if (*argv == NULL)
        emit_help();

    while (*argv != NULL) {
        c = *argv;
        while (*c != '\0') {
            printf("%3c", isprint(*c) ? *c : '?');
            c++;
        }
        putchar('\n');

        c = *argv;
        while (*c != '\0')
            printf("%3x", *c++ & 255);
        putchar('\n');

        argv++;
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fputs("Usage: whatargs arg [..]\n", stderr);
    exit(EX_USAGE);
}
