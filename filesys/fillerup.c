/* fillerup - fills up a filesystem */

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define MAX_BACKOFF 32          /* seconds, for write failure conditions */

void do_close(int sig);
void emit_help(void);

int fd;                         /* file descriptor */
volatile int done_writing;      /* flag to abort infinite loop on signal */

int Flag_Quiet;

int main(int argc, char *argv[])
{
    char buf[] =
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa ";
    size_t buf_size = sizeof(buf);
    int ch;                     /* getopt */
    unsigned int backoff = 1;   /* write failure delay (seconds) */
    struct sigaction act;       /* SIGUSR1 */

#ifdef __OpenBSD__
    if (pledge("cpath stdio wpath", NULL) == -1)
        err(1, "pledge failed");
#endif

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

    if (!*argv || *argv[0] == '\0')
        emit_help();

    if (strnlen(*argv, PATH_MAX) >= PATH_MAX)
        errx(EX_DATAERR, "filename exceeds PATH_MAX");

    if ((fd =
         open(argv[0], O_APPEND | O_CREAT | O_EXCL | O_NOFOLLOW | O_WRONLY,
              S_IRUSR | S_IWUSR)) < 0)
        err(EX_IOERR, "open failed");

    act.sa_handler = do_close;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(SIGUSR1, &act, NULL) != 0)
        err(EX_OSERR, "sigaction failed");

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
    if (close(fd) < 0)
        err(EX_IOERR, "close() error");
}

void emit_help(void)
{
    fputs("Usage: fillerup [-q] filename\n", stderr);
    exit(EX_USAGE);
}
