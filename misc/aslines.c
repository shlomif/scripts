/* aslines - similar to ZSH `print -l ...`; mostly used with entr(1)
 * which needs lines of files on standard input:
 *
 *   aslines lib/Data/SSHPubkey.pm t/10-pubkeys.t | entr pmt
 *
 * in one terminal while those files are being edited in another */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sysexits.h>

int main(int argc, char *argv[])
{
#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif
    if (argc < 2) {
        fputs("Usage: aslines text ..\n", stderr);
        exit(EX_USAGE);
    }
    argv++;
    while (*argv) {
        puts(*argv);
        argv++;
    }
    exit(EXIT_SUCCESS);
}
