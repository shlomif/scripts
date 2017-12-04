/* sgrax - reverse of xargs */

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, fd[2], status = 1;
    pid_t pid;
    size_t amount;

    while ((ch = getopt(argc, argv, "h?")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 2)
        emit_help();

    if (pipe(fd) == -1)
        err(EX_OSERR, "pipe failed");

    if ((pid = fork()) < 0)
        err(EX_OSERR, "fork failed");

    if (pid == 0) {             /* child */
        close(fd[1]);
        if (fd[0] != STDIN_FILENO) {
            if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO)
                err(EX_OSERR, "dup2 failed");
            close(fd[0]);
        }
        if (execlp(*argv, *argv, (char *) 0) == -1)
            err(1, "could not exec '%s'", *argv);
        /* NOTREACHED */
    }

    /* parent */
    close(fd[0]);
    argv++;
    while (*argv != NULL) {
        amount = strlen(*argv);
        /* this is system call heavy, probably more efficient to write
         * to a buffer and write buf when that full or argv empty... */
        write(fd[1], *argv, amount);
        write(fd[1], " ", (size_t) 1);
        argv++;
    }
    close(fd[1]);
    if (waitpid(pid, &status, 0) < 0)
        err(EX_OSERR, "waitpid failed");
    if (status != 0)
        exit(1);
    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: sgrax command arg1 [arg2 ..]");
    exit(EX_USAGE);
}
