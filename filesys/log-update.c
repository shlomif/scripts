/* log-update - git post-commit or post-merge hook that writes the path
 * of the repository to a file; this file is used by other tools e.g.
 * ones that push the repositories off to other systems, etc */

#include <sys/file.h>
#include <sys/param.h>

#include <err.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define UPDATE_FILE ".git-updates"

char pwd[PATH_MAX + 1];

int main(void)
{
    char *home, *update_file;
    int fd;
    sigset_t block;
    size_t len;
    ssize_t ret;

    if ((home = getenv("HOME")) == NULL)
        err(EX_OSERR, "no HOME");
    if (asprintf(&update_file, "%s/%s", home, UPDATE_FILE) == -1)
        err(EX_OSERR, "asprintf() failed");

    sigemptyset(&block);
    sigaddset(&block, SIGINT);
    if (sigprocmask(SIG_BLOCK, &block, NULL) == -1)
        err(EX_OSERR, "sigprocmask failed %d", block);

    if ((fd = open(update_file, O_APPEND | O_CREAT | O_WRONLY, 0666)) == -1)
        err(EX_IOERR, "open failed '%s'", UPDATE_FILE);
    if (flock(fd, LOCK_EX) == -1)
        err(EX_IOERR, "flock failed");

    if (getcwd((char *) &pwd, PATH_MAX) == NULL)
        err(EX_IOERR, "getcwd failed");
    len = strnlen((const char *) &pwd, PATH_MAX) + 1;
    ret = write(fd, &pwd, len);
    if (ret == -1) {
        err(EX_IOERR, "write failed");
    } else if (ret != len) {
        err(EX_IOERR, "incomplete write?? want %lu got %ld", len, ret);
    }

    flock(fd, LOCK_UN);
    close(fd);

    exit(0);
}
