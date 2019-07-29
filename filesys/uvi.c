/* uvi - edit files without changing the access times */

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

enum { NOPE = -1, CHILD };

struct filetimes {
    char *file;
    struct timespec when[2];
};

int main(int argc, char *argv[])
{
    char **ap;
    int exit_status = 0, filenum;
    pid_t pid;
    struct filetimes *ft, **restore;
    struct stat sb;

    if (argc == 1) {
        fprintf(stderr, "Usage: uvi file [file2 ..]\n");
        exit(EX_USAGE);
    }

    if ((restore = calloc(argc - 1, sizeof(struct filetimes *))) == NULL)
        err(EX_OSERR, "calloc failed");

    ap = &argv[1];
    filenum = 0;
    while (*ap != NULL) {
        if (stat(*ap, &sb) != 0)
            err(EX_OSERR, "stat failed");
        if (!S_ISREG(sb.st_mode))
            err(1, "not a file: %s", *ap);
        if ((ft = malloc(sizeof(struct filetimes))) == NULL)
            err(EX_OSERR, "malloc failed");
        ft->file = *ap;
        ft->when[0] = sb.st_atim;
        ft->when[1] = sb.st_mtim;
        restore[filenum++] = ft;
        ap++;
    }

    pid = fork();
    if (pid == NOPE)
        err(EX_OSERR, "fork failed");

    if (pid == CHILD) {
        char *ed;
        ed = getenv("VISUAL");
        if (ed == NULL || *ed == '\0') {
            ed = getenv("EDITOR");
            if (ed == NULL || *ed == '\0')
                ed = "vi";
        }
        argv[0] = ed;
        execvp(ed, argv);
        err(1, "could not exec '%s'", ed);
    } else {
        int status;
        if (wait(&status) == -1)
            err(EX_OSERR, "wait failed");
        if (status != 0)
            exit_status = 1;
    }

    for (int i = 0; i < filenum; i++) {
        if (utimensat(AT_FDCWD, restore[i]->file, restore[i]->when, 0) != 0) {
            warn("could not restore %s to original time", restore[i]->file);
            exit_status = 1;
        }
    }

    exit(exit_status);
}
