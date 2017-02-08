/* Makes a linux loopback image. Because I keep forgetting the steps. */

#define _BSD_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <bsd/stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <glob.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define WRITE_BUF_SIZE 8192

const char *loopdevs = "/dev/loop*";

int Flag_ZeroFile;              // -Z

void emit_help(void);
int losetup(const char *device, const char *path);

int main(int argc, char *argv[])
{
    int ch;

    char *filename;
    int fd;
    off_t curfilesize;

    off_t newfilesize;
    const char *errstr = NULL;

    glob_t lodevs;
    size_t glnum;

    char *writebuf;
    off_t delta;
    int ret;

    while ((ch = getopt(argc, argv, "h?Z")) != -1) {
        switch (ch) {
        case 'Z':
            Flag_ZeroFile = 1;
            break;
        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 2)
        emit_help();

    filename = *argv;
    argv++;
    // KLUGE no "off_t max" so uh guess that LONG_MAX okay. Min is also
    // likely too small for anything productive, but meh.
    newfilesize = (off_t) strtonum(*argv, 1, LONG_MAX, &errstr);
    if (errstr)
        errx(1, "could not strtonum(3) file-size '%s': %s", *argv, errstr);

    if ((fd = open(filename, O_CREAT | O_RDWR, 0666)) == -1)
        err(EX_IOERR, "could not open '%s'", filename);
    if ((curfilesize = lseek(fd, 0, SEEK_END)) == -1)
        err(EX_IOERR, "seek failed on '%s'", filename);

    if (newfilesize != curfilesize) {
        if (ftruncate(fd, newfilesize) != 0)
            err(EX_IOERR, "ftruncate failed on '%s'", filename);
    }
    if (Flag_ZeroFile) {
        if ((writebuf = calloc(WRITE_BUF_SIZE, 1)) == NULL)
            err(EX_OSERR, "calloc failed");
        curfilesize = 0;
        while (curfilesize < newfilesize) {
            delta = newfilesize - curfilesize;
            if (delta > WRITE_BUF_SIZE)
                delta = WRITE_BUF_SIZE;
            if ((ret = pwrite(fd, writebuf, delta, curfilesize)) == -1)
                err(EX_OSERR, "pwrite failed");
            if (ret != delta)
                errx(EX_OSERR, "incomplete write");
            curfilesize += delta;
        }
    }
    close(fd);

    glob(loopdevs, GLOB_NOSORT, NULL, &lodevs);
    for (glnum = 0; glnum < lodevs.gl_pathc; glnum++) {
        if (losetup(lodevs.gl_pathv[glnum], filename)) {
            printf("%s\n", lodevs.gl_pathv[glnum]);
            exit(EXIT_SUCCESS);
        }
    }
    exit(1);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: makeloimage [-Z] file file-size\n");
    exit(EX_USAGE);
}

int losetup(const char *device, const char *path)
{
    pid_t child;
    int status;
    child = vfork();
    if (child == 0) {           // child
        execlp("losetup", "losetup", device, path, (char *) NULL);

    } else if (child > 0) {     // parent ok
        if (waitpid(-1, &status, 0) == -1)
            err(EX_OSERR, "waitpid failed");
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
            return 1;
    } else {                    // parent not ok
        err(EX_OSERR, "could not vfork");
    }
    return 0;
}
