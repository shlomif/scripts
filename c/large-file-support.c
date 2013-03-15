/*
 * Test *lack* of Linux Large File Support (LFS). Had to compile on a
 * RedHat 7.3 Linux system, as I could not convince a RHEL5 system to
 * compile something that could not write beyond the 2**32/2-1 limit.
 *
 * This program appends to the 'output' file. A mktemp(1) directory may
 * be prudent on a mixed-user test system.
 *
 * fflush() threw the error during testing - other programs that lack error
 * checking on fflush may "hang" (strace should show write calls failing?)
 * or perhaps crash.
 *
 */

#include <stdio.h>
#include <limits.h>

int main()
{
    FILE *fh;
    int i;

    if ((fh = fopen("output", "a")) == NULL) {
        perror("could not open 'output' file");
        return (1);
    }
    for (i = 0; i < INT_MAX; i++) {
        if (fprintf(fh, "%d\n", i) < 0) {
            perror("fprintf() error");
            goto cleanup;
        }
        if (i % 4096 == 0) {
            if (fflush(fh) != 0) {
                perror("fflush() error");
                goto cleanup;
            }
        }
    }
  cleanup:
    if (fclose(fh) != 0) {
        perror("error closing output");
        return (1);
    }
    return (0);
}
