/* nodupenv - disallow duplicate environment variables past the first */

#include <stdio.h>
#include "redblack-bst.h"

extern char **environ;

int main(int argc, char *argv[])
{
    char **envp, *envline, *envname, *orig;

    if (argc < 2) {
        fprintf(stderr, "Usage: nodupenv command [args ..]\n");
        exit(EX_USAGE);
    }
    argv++;                     /* move past our name for exec */

    envp = environ;
    while (*envp != NULL) {
        orig = envline = strdup(*envp);
        if (envline == NULL) {
            err(EX_OSERR, "strdup failed on '%s'", *envp);
        }
        envname = strsep(&envline, "=");
        if (envname == NULL) {
            err(EX_OSERR, "strsep failed on '%s'", *envp);
        }

        rbbst_add(envname, *envp);

        free(orig);
        envp++;
    }

    environ = rbbst2envp();

    execvp(*argv, argv);
    err(EX_OSERR, "could not exec '%s'", *argv);
}
