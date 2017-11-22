/* Experiment with manually changing the environ(7) pointer to include
 * duplicate entries. View the results with e.g.
 *
 *   muss-with-environ env | grep FOO
 *
 * As to why any application would do such, well, that's a good
 * question, and I've certainly never seen it in production, but someone
 * on freenode #zsh claimed to have seen duplicate environment entries
 * on Mac OS X.
 *
 * ... and it turns out that shells do different things, depending
 *
 *  $ ./muss-with-environ  zsh   -c 'echo $FOO' 
 *  bar
 *  $ ./muss-with-environ  bash  -c 'echo $FOO'
 *  zot
 *  $ 
 *
 * Given the bash-uses-second behavior, if a program sanitizes only the
 * first instance of a variable, an evil second PATH or LD_RUN_PATH or
 * whatever could be run through a buggy version of sudo or equivalent
 * and then hey presto security flaw. sudo has fixed this issue:
 *
 * https://www.sudo.ws/repos/sudo/rev/d4dfb05db5d7
 *
 * Maybe libc itself should clean up duplicates so each and every
 * program need not worry about this?
 *
 * https://sourceware.org/bugzilla/show_bug.cgi?id=19749
 */

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// no limit in the C spec so fake one
#define ENV_COUNT_MAX 8192

extern char **environ;

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch, envc = 0;
    char **newenv;

    while ((ch = getopt(argc, argv, "h?")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 1)
        emit_help();

    // figure out how many env pointers there are...
    newenv = environ;
    while (*newenv++ != NULL)
        envc++;

    if (envc > ENV_COUNT_MAX)
        errx(1, "too many environment variables");

    // NOTE cannot realloc environ, as by default it resides in the stack
    // and not on the heap
    if ((newenv = malloc(sizeof(char *) * (envc + 3))) == NULL)
        err(EX_OSERR, "could not malloc new environ");
    if (envc > 0)
        memcpy(newenv, environ, sizeof(char *) * envc);

    newenv[envc] = "FOO=bar";
    newenv[envc+1] = "FOO=zot";
    newenv[envc+2] = NULL;

    environ = newenv;
    execvp(*argv, argv);

    /* NOTREACHED unless exec fails */
    err(EX_OSERR, "could not exec '%s'", *argv);
    exit(1);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: muss-with-environ command [args ..]\n");
    exit(EX_USAGE);
}
