/* waiter - waits for something and reports the exit status
 *
 * movtivated by the curious case of the missing segfault (or bus
 * error) message:
 *
 *   $ ./segfault
 *   zsh: bus error  ./segfault
 *   $ ssh localhost ./segfault
 *   $ ssh localhost waiter ./segfault
 *   waiter: child exited with signal 10
 *
 * (be sure to check that exit code from SSH) */

#include <sys/wait.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

enum { NOPE = -1, CHILD };

int main(int argc, char *argv[])
{
    int status;
    pid_t pid;

#ifdef __OpenBSD__
    if (pledge("exec proc stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    if (argc < 2) {
        fputs("Usage: waiter command [args ..]\n", stderr);
        exit(EX_USAGE);
    }

    if ((pid = fork()) == NOPE)
        err(EX_OSERR, "fork failed");

    if (pid == CHILD) {
        argv++;
        execvp(*argv, argv);
        err(EX_OSERR, "exec failed");
    } else {
        if (waitpid(pid, &status, 0) < 0)
            err(EX_OSERR, "could not waitpid");
        if (WIFEXITED(status)) {
            exit(WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            warnx("child exited with signal %d", WTERMSIG(status));
            exit(128 + WTERMSIG(status));
        } else {
            err(EX_OSERR, "unknown waitpid condition?? status=%d", status);
        }
    }
    /* NOTREACHED */
    exit(1);
}
