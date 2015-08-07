/*
 * Fills up a filesystem. Yes, there are use cases for such code. Like
 * when /var is 100% full (or more, or sometimes negative, according to
 * the OS and df(1) involved, and whether the user that filled the
 * partition was root or not) yet du(1) or find(1) only show much less
 * than available space consumed, or at least when one needs to recreate
 * and test such a condition, as opposed to learning it live in
 * production like I did (in particular, via the rm(1) of the file that
 * is still being written to).
 *
 *   fillerup [-h] [-q] filename
 *
 * The supplied filename will be created and written to. A USR1 signal
 * will cause the attempted close(2) of the file descriptor. The -q flag
 * will disable the default printing of the PID (which is otherwise
 * handy to pass to `kill -USR1 ...`). All output is to stderr, and
 * usually only when something is awry.
 *
 * Old system portability: use strlen, extirpate O_NOFOLLOW, maybe more,
 * depending on signal handling or other things I do not know about.
 *
 * Disabling the error-on-write might also be profitable, if one does
 * not care to see that.
 *
 * Some testing results:
 *
 *   * OpenBSD 5.2 shows 105% full and -89632 Avail after the root
 *     reserved space is run through. (Otherwise "0" and 100%.)
 *   * A Mac OS X disk image (extended filesystem) offers no root
 *     reserved space.
 *   * An old RedHat Linux 7.3 virt shows 100% full, even after the root
 *     reserved space is run int. (And "0" Avail.)
 *
 *  Other things to test:
 *
 *   * Sparse files and how various things view how much space those use
 *     (mess around with lseek(2) calls).
 *   * rm the file being written to and find what various utilities can
 *     show the unlinked but as-yet active inode.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define MAX_BACKOFF 32          /* seconds, for write failure conditions */

/* Ugh. */
#if __APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070
static inline size_t strnlen(const char *__string, size_t __maxlen)
{
    int len = 0;
    while (__maxlen-- && *__string++)
        len++;
    return len;
}
#endif

void do_close(int sig);
void emit_help(void);

int fd;                         /* file descriptor */
int done_writing;               /* flag to abort infinite loop on signal */

int Flag_Quiet;

int main(int argc, char *argv[])
{
    char buf[] =
        "all writes and no planning make filesystem go something something... ";
    unsigned long buf_size = sizeof(buf);
    int ch;                     /* getopt */
    unsigned int backoff = 1;   /* write failure delay (seconds) */
    struct sigaction act;       /* SIGUSR1 */

    while ((ch = getopt(argc, argv, "hq")) != -1) {
        switch (ch) {
        case 'h':
            emit_help();
            /* NOTREACHED */
        case 'q':
            Flag_Quiet = 1;
            break;
        default:
            ;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc == 0 || argv[0] == NULL)
        emit_help();

    /* could also check if filename is "" but open(2) barfs on that so meh */
    if (strnlen(argv[0], PATH_MAX) >= PATH_MAX)
        errx(EX_DATAERR, "filename exceeds PATH_MAX");

    /* XXX need to learn me the mask stuff better */
    if ((fd =
         open(argv[0], O_APPEND | O_CREAT | O_EXCL | O_NOFOLLOW | O_WRONLY,
              S_IRUSR | S_IWUSR)) < 0)
        err(EX_IOERR, "open() error for '%s'", argv[0]);

    act.sa_handler = do_close;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(SIGUSR1, &act, NULL) != 0)
        err(EX_OSERR, "sigaction() error");

    /*
     * Or just use a wrapper:
     *
     *   #!/bin/sh
     *   echo $$
     *   exec fillerup -q
     */
    if (!Flag_Quiet)
        fprintf(stderr, "pid %ld\n", (long int) getpid());

    while (done_writing != 1) {
        if (write(fd, buf, buf_size) < 0) {
            if (!Flag_Quiet)
                warn("write() error");
            sleep(backoff);
            backoff <<= 1;
            if (backoff > MAX_BACKOFF)
                backoff = MAX_BACKOFF;
        } else {
            backoff = 1;
        }
    }

    exit(EXIT_SUCCESS);
}

void do_close(int sig)
{
    done_writing = 1;
    /*
     * XXX disable the sleep related code to see if that changes whether
     * this call emits an error? - nope. no error from close.
     */
    if (close(fd) < 0)
        err(EX_IOERR, "close() error");
}

void emit_help(void)
{
    fprintf(stderr, "Usage: fillerup [-h] [-q] filename");
    exit(EX_USAGE);
}
