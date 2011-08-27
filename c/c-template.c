/*
 * Blah blah blah
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

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



    exit(EXIT_SUCCESS);
}
