/* Copies standard input to standard output and to a clipboard command. */

#include <sys/wait.h>

#include <err.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define BUFSIZE 4096

/* This is for Mac OS X; see the man page for details on creating a
 * platform specific wrapper as necessary for xsel or the like. Or,
 * adjust the execlp call below to call the appropriate program
 * directly, though bear in mind xsel(1) may do the Wrong Thing if
 * called directly under vi(1). */
#define CLIPBOARD "pbcopy"

void emit_help(void);

int main(int argc, char *argv[])
{
    char buf[BUFSIZE], *clippy, *shortname;
    int ch, fd[2];
    pid_t pid;
    sigset_t blockthese;
    ssize_t howmuch;

    while ((ch = getopt(argc, argv, "h?")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }

    if (pipe(fd) == -1)
        err(EX_OSERR, "pipe() failed");

    /* Ignore control+c generated signal to the foreground process
     * group; this allows one to collect dynamic output such as `ping
     * ... | copycat` as otherwise the INT results in no clipboard data
     * being set. NOTE may be too clever for own good. */
    sigemptyset(&blockthese);
    sigaddset(&blockthese, SIGINT);
    if (sigprocmask(SIG_BLOCK, &blockthese, NULL) == -1)
        err(EX_OSERR, "could not set sigprocmask of %d", blockthese);

    if ((pid = vfork()) < 0) {
        err(EX_OSERR, "fork() failed");

    } else if (pid > 0) {       /* parent */
        close(fd[0]);

        while (1) {
            if ((howmuch = read(STDIN_FILENO, &buf, (size_t) BUFSIZE)) < 0) {
                err(EX_IOERR, "read() failed");
            } else if (howmuch == 0) {  /* EOF */
                break;
            }
            if (write(fd[1], &buf, (size_t) howmuch) == -1)
                err(EX_IOERR, "write() to clipboard command failed");
            if (write(STDOUT_FILENO, &buf, (size_t) howmuch) == -1)
                err(EX_IOERR, "write() to STDOUT failed");
        }
        close(fd[1]);
        if (waitpid(pid, NULL, 0) < 0)
            err(EX_OSERR, "waitpid() failed");

    } else {                    /* child */
        close(fd[1]);

        if (fd[0] != STDIN_FILENO) {
            if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO)
                err(EX_OSERR, "dup2() to STDIN failed");
            close(fd[0]);
        }

        if ((clippy = getenv("CLIPBOARD")) == NULL)
            clippy = CLIPBOARD;
        if ((shortname = strrchr(clippy, '/')) != NULL)
            shortname++;
        else
            shortname = clippy;

        if (execlp(clippy, shortname, (char *) 0) < 0) {
            warn("execlp() of %s failed", clippy);
            _exit(EX_OSERR);    /* but really the parent eats a SIGPIPE */
        }
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: ... | copycat\n");
    exit(EX_USAGE);
}
