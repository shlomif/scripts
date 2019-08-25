/* charinfo - display different representations for a given ascii(7) char */

#include <err.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);

int main(int argc, char *argv[])
{
    char *sp;
    int ch, ret;

#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    argv++;
    while (*argv) {
        ret = sscanf(*argv, "%4i", &ch);
        if (ret == 0) {
            ret = sscanf(*argv, "%1c", (char *) &ch);
            if (ret == 0)
                errx(1, "unknown input: %s", *argv);
        } else {
            /* TODO if not ascii(7) maybe lob to some Unicode tool */
            if (ch < 0 || ch > 0x7f)
                errx(1, "input out of range: %s", *argv);
        }

        if (isprint(ch))
            asprintf(&sp, "'%c'", ch);
        else
            asprintf(&sp, ".");

        printf("\\%03o\t\\x%02x\t%d\t%s\n", ch, ch, ch, sp);

        free(sp);
        argv++;
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fputs("Usage: charinfo char-or-integer ..\n", stderr);
    exit(EX_USAGE);
}
