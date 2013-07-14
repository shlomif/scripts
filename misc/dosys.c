/*
 * Experiment with shell quoting needs for the system(3) call. If at
 * all possible, instead just avoid the shell, and use an exec* or some
 * other call. Also, this code assumes that a non-malicious user is
 * using the code, is actually allowed to use the code, etc, etc, etc.
 *
 * The gist of the problem is to distinguish 'ls "commit log"' for
 * cases where there is a file with a space in its name, from 'git
 * commit log' for cases where the file is 'log' without any spaces in
 * its name. If these examples are not atomized into two (ls, filename)
 * or three (git, git command, filename) distinct elements... well,
 * best of luck. env(1) just does a execvp(*argv, argv) on the atoms
 * as-is, for example.
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

/* Other considerations might be that no path exceeds MAXPATHLEN from
 * the system include defines, but that's yet more work.
 */
#define MAX_CMD_LEN 1024

/* flags */
int f_dryrun;

int main(int argc, char *argv[])
{
    int ch, argidx, cmdidx, sysval;
    unsigned int i;
    size_t arglen;
    char cmd_buf[MAX_CMD_LEN];

    while ((ch = getopt(argc, argv, "n")) != -1) {
        switch (ch) {
        case 'n':
            f_dryrun = 1;
            break;
        default:
            ;
        }
    }
    argc -= optind;
    argv += optind;

    /*
     * So this approach just doublequotes each atom (supplied via
     * argv), which should solve the "spaces in file path" problem but
     * probably remains open to any number of other problems. Might be
     * better to use some printf or other string function instead of
     * the char-by-char foo?
     */
    cmd_buf[0] = '\0';
    cmdidx = 0;
    for (argidx = 0; argidx < argc; argidx++) {
        if (argidx > 0) {
            cmd_buf[cmdidx++] = ' ';
            if (cmdidx >= MAX_CMD_LEN)
                errx(EX_DATAERR, "input exceeds command length (%d)",
                     MAX_CMD_LEN);
        }
        cmd_buf[cmdidx++] = '"';
        if (cmdidx >= MAX_CMD_LEN)
            errx(EX_DATAERR, "input exceeds command length (%d)",
                 MAX_CMD_LEN);

        arglen = strlen(argv[argidx]);
        for (i = 0; i < arglen; i++) {
            /*
             * Of course, doublequotes within the thus doublequoted atom
             * must then be escaped, though this might cause problems if
             * the doublequote is already escaped, but by that point the
             * Stygian swamp is already sucking you in, godspeed. 
             */
            switch (argv[argidx][i]) {
            case '"':
                cmd_buf[cmdidx++] = '\\';
                if (cmdidx >= MAX_CMD_LEN)
                    errx(EX_DATAERR, "input exceeds command length (%d)",
                         MAX_CMD_LEN);
            default:
                cmd_buf[cmdidx++] = argv[argidx][i];
                if (cmdidx >= MAX_CMD_LEN)
                    errx(EX_DATAERR, "input exceeds command length (%d)",
                         MAX_CMD_LEN);
            }
        }

        cmd_buf[cmdidx++] = '"';
        if (cmdidx >= MAX_CMD_LEN)
            errx(EX_DATAERR, "input exceeds command length (%d)",
                 MAX_CMD_LEN);
    }
    cmd_buf[cmdidx] = '\0';

    if (f_dryrun) {
        warnx("command is [%s]", cmd_buf);

    } else {
        if (fflush(stdout) != 0)
            err(EX_IOERR, "could not fflush(stdout)");
        sysval = system(cmd_buf);
        if (sysval != 0)
            err(1, "error running command (%d)", sysval);
    }

    exit(EXIT_SUCCESS);
}
