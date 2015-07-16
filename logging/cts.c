/*
 * Like ts(1) of moreutils fame, only in C for C practice, and only
 * shows the delta between subsequent log lines (use ts(1) if such 
 * customization is necessary). The following should be roughly
 * equivalent:
 *
 *   ... | cts
 *   ... | ts -i '%.s'
 */

#include <err.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#define NSEC_IN_SEC 1000000000

#define NUMFDS 1

struct timespec when;
long double now, prev;

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;

    bool firstpost = true;
    bool needts = true;
    char buf[BUFSIZ];
    struct pollfd fdwatch[NUMFDS];
    int fdready, readret;

    /* Unbuffered output by default (ts(1) does this) to minimize risk of
     * log lossage should something crash. Use -l for line-buffered. */
    setvbuf(stdout, (char *)NULL, _IONBF, (size_t) 0);

    while ((ch = getopt(argc, argv, "h?l")) != -1) {
        switch (ch) {

        case 'l':
            setvbuf(stdout, (char *)NULL, _IOLBF, (size_t) 0);
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

    fdwatch[0].fd = STDIN_FILENO;
    fdwatch[0].events = POLLIN;

    while (1) {
        fdready = poll((struct pollfd *) &fdwatch, (nfds_t) NUMFDS, INFTIM);
        if (fdready == -1)
            err(EX_IOERR, "poll() failed");
        if (fdready == 0)
            continue;           // timeout (???), try again
        if ((fdwatch[0].revents & (POLLERR | POLLNVAL)))
            errx(EX_IOERR, "naughty fd %d", fdwatch[0].fd);
        if ((fdwatch[0].revents & (POLLIN | POLLHUP))) {
            readret = read(STDIN_FILENO, buf, sizeof(buf));
            if (readret == -1)
                err(EX_IOERR, "read() failed");
            if (readret == 0)
                exit(EXIT_SUCCESS);     // EOF

            for (ssize_t i = 0; i < readret; i++) {
                switch (buf[i]) {
                case '\n':
                    putchar('\n');
                    needts = true;
                    break;

                default:
                    if (needts) {
                        if (clock_gettime(CLOCK_MONOTONIC, &when) == -1)
                            err(EX_OSERR, "clock_gettime() failed");
                        now =
                            when.tv_sec +
                            when.tv_nsec / (long double) NSEC_IN_SEC;
                        printf("%.6Lf ", firstpost ? 0.0 : now - prev);
                        prev = now;
                        needts = false;
                        firstpost = false;
                    }
                    putchar(buf[i]);
                }
            }
        }
    }

    exit(1);                    // *NOTREACHED*
}

void emit_help(void)
{
    fprintf(stderr, "Usage: ... | cts [-l]\n");
    exit(EX_USAGE);
}
