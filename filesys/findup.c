/* findup - search for a directory that contains a given filename */

#include <sys/stat.h>
#include <sys/types.h>

#include <err.h>
#include <getopt.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define EX_NOT_FOUND 1

enum { FINDUP_READABLE, FINDUP_FILE, FINDUP_DIR };

int Flag_NoEscapeHome;          /* -H */
int Flag_TestFor;               /* -d or -f */
int Flag_Quiet;                 /* -q */

int does_match(char *wanted, char *in_dir);
void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;
    char *homedir, *search_path, *sp;

#ifdef __OpenBSD__
    if (pledge("rpath stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    while ((ch = getopt(argc, argv, "Hdfh?q")) != -1) {
        switch (ch) {
        case 'H':
            Flag_NoEscapeHome = 1;
            break;
        case 'd':
            Flag_TestFor = FINDUP_DIR;
            break;
        case 'f':
            Flag_TestFor = FINDUP_FILE;
            break;
        case 'q':
            Flag_Quiet = 1;
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

    if (argc == 0)
        emit_help();

    /* special cases for absolute path searches */
    if ((*argv)[0] == '/') {
        if (((*argv)[1] == '\0' && Flag_TestFor != FINDUP_FILE)
            || does_match(*argv, "")) {
            if (!Flag_Quiet)
                write(STDOUT_FILENO, "/\n", (size_t) 2);
            exit(EXIT_SUCCESS);
        }
        exit(EX_NOT_FOUND);
    }

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
        if (!homedir)
            errx(EX_OSERR, "could not determine HOME directory");
        if ((homedir = realpath(homedir, NULL)) == NULL)
            err(EX_OSERR, "realpath failed to resolve HOME directory");
    }

    search_path = (argc == 2) ? argv[1] : getcwd(NULL, PATH_MAX);
    if ((search_path = realpath(search_path, NULL)) == NULL)
        err(EX_OSERR, "realpath failed to resolve search path");

    while (1) {
        if (does_match(*argv, search_path)) {
            if (!Flag_Quiet)
                puts(*search_path == '\0' ? "/" : search_path);
            exit(EXIT_SUCCESS);
        }
        if (Flag_NoEscapeHome && strncmp(search_path, homedir, PATH_MAX) == 0)
            break;
        if ((sp = strrchr(search_path, '/')) == NULL)
            break;
        /* NOTE this will make search_path "" for the final iteration */
        *sp = '\0';
    }

    exit(EX_NOT_FOUND);
}

int does_match(char *wanted, char *in_dir)
{
    char *path;
    int found = 0;
    struct stat statbuf;

    if (asprintf(&path, "%s/%s", in_dir, wanted) < 0)
        err(EX_OSERR, "asprintf failed");
    if (stat(path, &statbuf) == -1)
        goto DONEHERE;

    switch (Flag_TestFor) {
    case FINDUP_READABLE:
        /* well, the stat did find something... */
        found = 1;
        break;
    case FINDUP_FILE:
        found = S_ISREG(statbuf.st_mode);
        break;
    case FINDUP_DIR:
        found = S_ISDIR(statbuf.st_mode);
        break;
    default:
        err(1, "invalid testfor flag set ?? %d", Flag_TestFor);
    }

  DONEHERE:
    free(path);
    return found;
}

void emit_help(void)
{
    fputs("Usage: findup [-H] [-d|-f] [-q] filename [dir-path]", stderr);
    exit(EX_USAGE);
}
