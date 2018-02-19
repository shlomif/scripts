/* pcre-precompile - compiles a PCRE regex (see pcreprecompile(3)) */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#include <pcre.h>

int main(int argc, char *argv[])
{
    FILE *fh;

    int erroffset, rc, size;
    const char *error;
    pcre *re;

    if (argc < 2) {
        fprintf(stderr, "Usage: pcre-precompile regex [out-file]\n");
        exit(EX_USAGE);
    }

    if ((re = pcre_compile(argv[1], 0, &error, &erroffset, NULL)) == NULL)
        errx(EX_DATAERR, "pcre_compile failed at %d: %s", erroffset, error);

    if ((rc = pcre_fullinfo(re, NULL, PCRE_INFO_SIZE, &size)) < 0)
        errx(1, "pcre_fullinfo failed (%d) ??", rc);

    /* full use would run along the lines of
     *   pcre *pattern = (pcre *)"...";
     * this is just the "..." portion of that to standard out
     */
    printf("\"");
    for (int i = 0; i < size; i++) {
        printf("\\x%02X", *((char *) re + i) & 255);
    }
    printf("\"");

    if (argc == 3) {
        if ((fh = fopen(argv[2], "w")) == NULL)
            err(EX_IOERR, "could not open '%s'", argv[2]);
        if ((rc = fwrite(re, 1, size, fh)) != size)
            errx(EX_IOERR, "fwrite failed (wrote %d expected %d)", rc, size);
        if (fclose(fh) != 0)
            err(EX_IOERR, "fclose failed");
    }

    exit(0);
}
