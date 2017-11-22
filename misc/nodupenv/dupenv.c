#if defined(linux) || defined(__linux) || defined(__linux__)
#define _BSD_SOURCE
#endif

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// this mostly to try to avoid overflows on malloc calls
#define MAXENV 134217728LU

extern char **environ;

int Flag_ClearEnv;              // -i

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    char **ep, **newenv = NULL;
    size_t i, envcount = 0;
    size_t newenv_count = 64;

    while ((ch = getopt(argc, argv, "h?i")) != -1) {
        switch (ch) {
        case 'i':
            Flag_ClearEnv = 1;
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

    if (argc < 1)
        emit_help();

    if (!Flag_ClearEnv) {
        ep = environ;
        while (*ep++ != NULL)
            envcount++;
        if (envcount > MAXENV)
            err(1, "environment variable count > %lu\n", MAXENV);
        if (envcount >= newenv_count)
            newenv_count = envcount << 1;
    }

    while (argc > 0) {
        if (strchr(*argv, '=') != NULL) {
            if (newenv == NULL) {
                if ((newenv = calloc(newenv_count, sizeof(char **))) == NULL)
                    err(EX_OSERR, "calloc failed");
                if (!Flag_ClearEnv) {
                    for (i = 0; i < envcount; i++) {
                        newenv[i] = environ[i];
                    }
                }
            }
            newenv[envcount++] = *argv;
            if (envcount >= newenv_count) {
                while (newenv_count < envcount) {
                    newenv_count <<= 1;
                    if (newenv_count > MAXENV)
                        err(1, "environment variable count > %lu\n", MAXENV);
                }
                if ((newenv =
                     realloc(newenv, newenv_count * sizeof(char **))) == NULL)
                    err(EX_OSERR, "realloc failed");
            }
            argc--;
            argv++;
        } else {
            /* non env=value argument, assume it is the program to exec */
            break;
        }
    }

    if (argc < 1)
        emit_help();

    if (newenv != NULL) {
        newenv[envcount] = (char *) NULL;
        environ = newenv;
    }

    execvp(*argv, argv);
    err(EX_OSERR, "could not exec '%s'", *argv);
    return 1;                   /* NOTREACHED */
}

void emit_help(void)
{
    fprintf(stderr, "Usage: dupenv [-i] [env=val ..] command [args ..]\n");
    exit(EX_USAGE);
}
