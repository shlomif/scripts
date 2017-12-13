/* nocolor - strip color codes from standard output and error */

#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include <pcre.h>

#define MAX_OFFSET 3

void emit_help(void);
void *handle_output(void *arg);

int main(int argc, char *argv[])
{
    int outp[2], errp[2];
    pid_t pid;
    pthread_t err_thread;
    int status;

    if (argc < 2)
        emit_help();

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
        if (execvp(*argv, argv) < 0)
            err(1, "could not exec '%s'", *argv);
        /* NOTREACHED */
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
    fprintf(stderr, "Usage: nocolor command [arg ..]");
    exit(EX_USAGE);
}

void *handle_output(void *arg)
{
    int *fds = arg;

    FILE *input, *output;
    char *line = NULL;
    size_t linebuflen = 0;
    ssize_t numchars;

    const char *error;
    int erroffset, rc;
    int options = 0;
    int offsets[MAX_OFFSET];
    pcre *re;

    int previous;

    if ((input = fdopen(*fds, "r")) == NULL)
        err(EX_IOERR, "fdopen failed fd=%d", *fds);
    if ((output = fdopen(*(fds + 1), "a")) == NULL)
        err(EX_IOERR, "fdopen failed fd=%d", *(fds + 1));

    if (*(fds + 1) == STDOUT_FILENO)
        setvbuf(output, (char *) NULL, _IONBF, (size_t) 0);

    /* regex built by ./nocolor-regex-gen */
    if ((re =
         pcre_compile
         ("(?:\033\\[(?:[578]|[34](?:[01234567]|8[:;](?:2[:;][0-9]{1,3}[;:][0-9]{1,3}[;:][0-9]{1,3}|5[:;][0-9]{1,10}))|10[01234567]|9[01234567]?|21?)m)+",
          0, &error, &erroffset, NULL)) == NULL)
        err(1, "pcre compile failed at offset %d: %s ??", erroffset, error);

    while ((numchars = getline(&line, &linebuflen, input)) > 0) {
        rc = pcre_exec(re, NULL, line, numchars, 0, 0, offsets, MAX_OFFSET);
        if (rc < 0) {
            /* no match in line: print as is */
            fwrite(line, numchars, (size_t) 1, output);
            continue;
        }
        /* should not happen as there should only be one substring */
        if (rc == 0)
            abort();

        if (offsets[0] != 0) {
            /* string begins with non-match material, print that */
            fwrite(line, offsets[0], (size_t) 1, output);
        }
        previous = offsets[1];

        while (1) {
            /* NOTE since the regex does not match the empty string the
             * PCRE_NOTEMPTY_ATSTART | PCRE_ANCHORED special case is not
             * used; see pcredemo.c */
            rc = pcre_exec(re, NULL, line, numchars, offsets[1], options,
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

            fwrite(line + previous, offsets[0] - previous, (size_t) 1, output);
            previous = offsets[1];
        }

        /* anything after the last match (possibly only the newline) */
        fwrite(line + previous, numchars - previous, (size_t) 1, output);
    }
    if (ferror(input))
        err(EX_IOERR, "error reading fd=%d", *fds);

    return (void *) 0;
}
