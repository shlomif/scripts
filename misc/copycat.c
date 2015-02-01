/*
 * Copies STDIN to both STDOUT and to the clipboard command defined by the
 * CLIPBOARD environment variable. This could also be done with a FIFO, which
 * may be more suitable in certain contexts:
 *
 *   mkfifo asdf
 *
 * And then:
 *
 *   $CLIPBOARD < asdf &
 *   tee asdf
 *
 * Assuming that the FIFO can be properly created, etc. Otherwise mostly
 * motivated by :%!pbcopy in vi annoyingly filtering out the thus copied data,
 * and the code is otherwise mostly lifted from APUE.
 *
 * See the copycat.1 file for additional documentation (or copy that file to a
 * man1 directory and then `man copycat`). X11-enabled vim users could instead
 * just use the ":%y *" or equivalent to interact with the X11 clipboard via
 * the special * register.
 */

#include <sys/wait.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define BUFSIZE 4096
/* This is for Mac OS X; see the man page for details on creating a platform
 * specific wrapper as necessary for xsel or the like. Or, adjust the execlp
 * call below to call the appropriate program directly, though bear in mind
 * xsel(1) may do the Wrong Thing if called directly under vi(1). */
#define CLIPBOARD "pbcopy"

int main(void)
{
    char buf[BUFSIZE], *clippy, *shortname;
    int fd[2];
    pid_t pid;
    ssize_t howmuch;

    if (pipe(fd) == -1)
        err(EX_OSERR, "pipe() failed");

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
