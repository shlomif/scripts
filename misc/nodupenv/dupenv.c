/* dupenv - duplicates environment variables */

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

/* this mostly to try to avoid overflows on malloc calls */
#define MAXENV 134217728LU

extern char **environ;

int Flag_KeepEnv = 1;           /* -i (borrowed from env(1)) */
int Flag_UnSafe;                /* -U (borrowed from perl(1)) */

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    char **ep, **newenv = NULL, *equalpos;
    size_t i, cur_env = 0;
    /* because on my Mac there's about 50 env variables set by default
     * (Centos7 has ~20 and OpenBSD ~10) be sure to update the tests if
     * this number is changed */
    size_t newenv_alloc = 64;

#ifdef __OpenBSD__
    if (pledge("exec stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    while ((ch = getopt(argc, argv, "h?iU")) != -1) {
        switch (ch) {
        case 'i':
            Flag_KeepEnv = 0;
            break;
        case 'U':
            Flag_UnSafe = 1;
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

    if (Flag_KeepEnv) {
        ep = environ;
        while (*ep++ != NULL)
            cur_env++;
        if (cur_env > MAXENV)
            err(1, "environment variable count > %lu\n", MAXENV);
        if (cur_env >= newenv_alloc) {
            newenv_alloc = cur_env << 1;
            if (newenv_alloc > MAXENV)
                err(1, "environment variable count > %lu\n", MAXENV);
        }
    }

    if ((newenv = calloc(newenv_alloc, sizeof(char **))) == NULL)
        err(EX_OSERR, "calloc failed");

    if (Flag_KeepEnv) {
        for (i = 0; i < cur_env; i++) {
            newenv[i] = environ[i];
        }
    }

    while (argc > 0) {
        if ((equalpos = strchr(*argv, '=')) != NULL) {
            if (equalpos == *argv && !Flag_UnSafe)
                errx(1, "invalid environment variable '%s'", *argv);
            newenv[cur_env++] = *argv;
            if (cur_env >= newenv_alloc) {
                while (newenv_alloc <= cur_env) {
                    newenv_alloc <<= 1;
                    if (newenv_alloc > MAXENV)
                        err(1, "environment variable count > %lu\n", MAXENV);
                }
                if ((newenv =
                     realloc(newenv, newenv_alloc * sizeof(char **))) == NULL)
                    err(EX_OSERR, "realloc failed");
            }
            argc--;
            argv++;
        } else {
            /* non env=value argument, assume it is the program to exec */
            break;
        }
    }

    newenv[cur_env] = (char *) NULL;
    environ = (char **) newenv;

    if (argc < 1) {
        for (i = 0; i < cur_env; i++) {
            puts(environ[i]);
        }
        exit(EXIT_SUCCESS);
    }

    execvp(*argv, argv);
    err(EX_OSERR, "could not exec '%s'", *argv);
    return 1;
}

void emit_help(void)
{
    fputs("Usage: dupenv [-i] [env=val ..] command [args ..]\n", stderr);
    exit(EX_USAGE);
}
