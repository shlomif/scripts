/* nocolor - strip color codes from standard output and error */

#include <sys/wait.h>

#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include <pcre.h>

#define MAX_OFFSET 3

/* NOTE this is also set in the unit tests */
#ifndef NOCOLOR_BUF_SIZE
#define NOCOLOR_BUF_SIZE 1024
#endif

/* one of the ISO-8613-3 can be 19 characters long (or possibly more
 * if 64-bit integers are supported), see nocolor-regex-gen for
 * commentary */
#define NOCOLOR_MAX_FRAG 18

pcre *re;

void emit_help(void);
void *handle_output(void *arg);

int main(int argc, char *argv[])
{
    int outp[2], errp[2];
    pid_t pid;
    pthread_t err_thread;
    int status;

    const char *error;
    int erroffset;

#ifdef __OpenBSD__
    if (pledge("exec proc stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    if (argc < 2)
        emit_help();

    /* regex built by ./nocolor-regex-gen */
    if ((re =
         pcre_compile
         ("(?:\033\\[(?:[34](?:8(?:[:;](?:2[:;][0-9]{1,3}[;:][0-9]{1,3}[;:][0-9]{1,3}|5[:;][0-9]{1,10})|;(?:[05678]|1(?:0[01234567])?|2[123456789]?|9[01234567]?|[34][0-9]?))|[01234567](?:;(?:[05678]|1(?:0[01234567])?|2[123456789]?|9[01234567]?|[34][0-9]?))?|9?;(?:[05678]|1(?:0[01234567])?|2[123456789]?|9[01234567]?|[34][0-9]?))|2(?:[23456789]?;(?:[05678]|1(?:0[01234567])?|2[123456789]?|9[01234567]?|[34][0-9]?)|1(?:;(?:[05678]|1(?:0[01234567])?|2[123456789]?|9[01234567]?|[34][0-9]?))?)?|1(?:0[01234567](?:;(?:[05678]|1(?:0[01234567])?|2[123456789]?|9[01234567]?|[34][0-9]?))?|;(?:[05678]|1(?:0[01234567])?|2[123456789]?|9[01234567]?|[34][0-9]?))|9(?:[01234567](?:;(?:[05678]|1(?:0[01234567])?|2[123456789]?|9[01234567]?|[34][0-9]?))?|;(?:[05678]|1(?:0[01234567])?|2[123456789]?|9[01234567]?|[34][0-9]?))?|[578](?:;(?:[05678]|1(?:0[01234567])?|2[123456789]?|9[01234567]?|[34][0-9]?))?|[06];(?:[05678]|1(?:0[01234567])?|2[123456789]?|9[01234567]?|[34][0-9]?))m)+",
          0, &error, &erroffset, NULL)) == NULL)
        err(1, "pcre compile failed at offset %d: %s ??", erroffset, error);

    if (pipe(outp) == -1)
        err(EX_OSERR, "pipe stdout failed");
    if (pipe(errp) == -1)
        err(EX_OSERR, "pipe stderr failed");
    if ((pid = fork()) < 0)
        err(EX_OSERR, "fork failed");

    if (pid == 0) {             /* child */
        close(outp[0]);
        if (outp[1] != STDOUT_FILENO) {
            if (dup2(outp[1], STDOUT_FILENO) != STDOUT_FILENO)
                err(EX_OSERR, "dup2 stdout failed");
            close(outp[1]);
        }
        close(errp[0]);
        if (errp[1] != STDERR_FILENO) {
            if (dup2(errp[1], STDERR_FILENO) != STDERR_FILENO)
                err(EX_OSERR, "dup2 stderr failed");
            close(errp[1]);
        }
        argv++;                 /* skip our name since not using getopt */
        execvp(*argv, argv);
        err(1, "could not exec '%s'", *argv);
    }

    /* parent */

    /* KLUGE probably too clever to reuse the closed fields for where
     * the output should be written to... */
    close(errp[1]);
    errp[1] = STDERR_FILENO;
    if (pthread_create(&err_thread, NULL, handle_output, &errp) != 0)
        err(EX_OSERR, "pthread_create failed");

    close(outp[1]);
    outp[1] = STDOUT_FILENO;
    handle_output(&outp);
    if (waitpid(pid, &status, 0) < 0)
        err(EX_OSERR, "waitpid failed");

    /* don't exit early should stderr still be being processed */
    pthread_join(err_thread, NULL);

    exit(status == 0 ? EXIT_SUCCESS : 1);
}

void emit_help(void)
{
    fputs("Usage: nocolor command [arg ..]\n", stderr);
    exit(EX_USAGE);
}

void *handle_output(void *arg)
{
    int *fds = arg;
    int previous;
    ssize_t numchars, remain;
    ssize_t leftover = 0;

    char *buf_offset, buf[NOCOLOR_BUF_SIZE + 1];

    int rc;
    int options = 0;
    int offsets[MAX_OFFSET];

    /* regex and writes are always done from buf[0]; offset from that is
     * for when there may be a match broken between reads */
    buf_offset = buf;
    /* ensure buffer is a "string" for strrchr */
    buf[NOCOLOR_BUF_SIZE] = '\0';
    while (1) {
        previous = 0;
        numchars = read(*fds, buf_offset, NOCOLOR_BUF_SIZE - leftover);
        if (numchars < 1) {
            switch (numchars) {
            case 0:            /* EOF */
                goto ALMOST_ALL_DONE;
            case -1:
                err(EX_IOERR, "read failed");
            default:
                errx(EX_IOERR, "read failed ??");
            }
        }
        numchars += leftover;

        rc = pcre_exec(re, NULL, buf, numchars, 0, 0, offsets, MAX_OFFSET);
        if (rc < 0) {
            /* no full match */
            goto LEFTOVER;
        }
        /* should not happen as there should only be one substring */
        if (rc == 0)
            abort();

        if (offsets[0] != 0) {
            /* string begins with non-match material, print that */
            write(*(fds + 1), buf, offsets[0]);
        }
        previous = offsets[1];

        while (1) {
            /* since the regex does not match the empty string the
             * PCRE_NOTEMPTY_ATSTART | PCRE_ANCHORED special case is not
             * used; see pcredemo.c */
            rc = pcre_exec(re, NULL, buf, numchars, offsets[1], options,
                           offsets, MAX_OFFSET);
            if (rc == PCRE_ERROR_NOMATCH) {
                /* all matches found */
                if (options == 0)
                    break;
                /* what usually happens here is +1 byte plus newline and
                 * unicode complications but again the regex does not
                 * match the empty string (nor does it span newlines) so
                 * in theory this should not be reached */
                errx(1, "unknown match condition %d ??", rc);
            }
            if (rc < 0)
                errx(1, "unrecoverable condition %d ??", rc);

            write(*(fds + 1), buf + previous, offsets[0] - previous);
            previous = offsets[1];
        }

        /* anything after the ultimate match (if any) complicated by the
         * possibility of a color code split across the buffer */
      LEFTOVER:
        remain = numchars - previous;
        if (remain > 0) {
            ssize_t tocheck;
            tocheck = remain > NOCOLOR_MAX_FRAG ? NOCOLOR_MAX_FRAG : remain;
            buf[numchars] = '\0';
            buf_offset = strrchr(buf + numchars - tocheck, '\033');
            if (buf_offset == NULL) {
                write(*(fds + 1), buf + previous, remain);
            } else {
                write(*(fds + 1), buf + previous,
                      buf_offset - (buf + previous));
                leftover = numchars - (buf_offset - buf);
                memmove(buf, buf_offset, leftover);
                buf_offset = buf + leftover;
                continue;
            }
        }
        buf_offset = buf;
        leftover = 0;
    }

  ALMOST_ALL_DONE:
    if (leftover > 0)
        write(*(fds + 1), buf, leftover);
    return (void *) 0;
}
