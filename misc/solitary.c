/* solitary - exec wrapper to dissociate from the parent; similar to the
 * code presented in "Complete Dissociation of Child from Parent" in
 * perlipc(1). a double fork, if necessary, would need to be added */

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd;
    pid_t pid;

    if (argc < 3) {
        fprintf(stderr, "Usage: solitary directory command [args ..]\n");
        exit(EX_USAGE);
    }

    argv++;
    if (chdir(*argv) == -1)
        err(EX_OSERR, "chdir failed");

    if ((fd = open("/dev/null", O_RDONLY)) == -1)
        err(EX_OSERR, "open failed");
    if (dup2(fd, STDIN_FILENO) == -1)
        err(EX_OSERR, "dup2 failed");
    if (close(fd) == -1)
        err(EX_OSERR, "close failed");

    if ((fd = open("/dev/null", O_WRONLY)) == -1)
        err(EX_OSERR, "open failed");
    if (dup2(fd, STDOUT_FILENO) == -1)
        err(EX_OSERR, "dup2 failed");
    if (close(fd) == -1)
        err(EX_OSERR, "close failed");

    if ((pid = fork()) == -1)
        err(EX_OSERR, "fork failed");
    if (pid > 0)
        exit(0);

    /* child */
    if (setsid() == -1)
        err(EX_OSERR, "setsid failed");

    if ((fd = dup(STDOUT_FILENO)) == -1)
        err(EX_OSERR, "dup failed");
    if (dup2(fd, STDERR_FILENO) == -1)
        err(EX_OSERR, "dup2 failed");
    if (close(fd) == -1)
        err(EX_OSERR, "close failed");

    argv++;
    execvp(*argv, argv);
    exit(EX_OSERR);
}
