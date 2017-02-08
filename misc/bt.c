/* bt, short for "backtrace" or a utility wrapper to call a debugger
 * (gdb by default) on the most recent *.core file (or failing that
 * executable) in the current working directory unless a core file or
 * program name is given; that is, much effort for a little automation. */

#include <sys/stat.h>

#include <dirent.h>
#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define HAS_X_BIT(m) ((((m) & S_IXUSR) == S_IXUSR) || (((m) & S_IXGRP) == S_IXGRP) || (((m) & S_IXOTH) == S_IXOTH))
#define MORE_RECENT(t,v) (t.tv_sec > v.tv_sec || (t.tv_sec == v.tv_sec && t.tv_nsec > v.tv_nsec))

char *core_of_prog(const char *filename);
void emit_help(void);
void find_core_or_prog(char **corefilep, char **prognamep);
char **parse_debugger_args(char *progname, char *corefile);
char *prog_of_core(const char *filename);

const char *Flag_Debugger = "gdb -q -tui";      // -D

int main(int argc, char *argv[])
{
    char *corefile = NULL;
    char **debugger_args, *env_tmp;
    char *progname = NULL;
    int ch;
    struct stat statbuf;

    env_tmp = getenv("BT_DEBUGGER");
    if (env_tmp != NULL)
        Flag_Debugger = env_tmp;

    while ((ch = getopt(argc, argv, "D:h?")) != -1) {
        switch (ch) {
        case 'D':
            Flag_Debugger = optarg;
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

    if (argc == 0) {
        find_core_or_prog(&corefile, &progname);
        if (progname == NULL)
            errx(1, "no *.core nor executable found in PWD");
    } else {
        progname = prog_of_core(*argv);
        if (progname != NULL) {
            corefile = *argv;
        } else {
            progname = *argv;
            corefile = core_of_prog(progname);
        }
        if (stat(corefile, &statbuf) != 0)
            corefile = NULL;
        if (stat(progname, &statbuf) != 0)
            err(EX_IOERR, "could not stat program '%s'", progname);
        if (!HAS_X_BIT(statbuf.st_mode))
            err(EX_IOERR, "program '%s' is not executable", progname);
    }

    debugger_args = parse_debugger_args(progname, corefile);

    if (execvp(*debugger_args, debugger_args) == -1)
        err(EX_OSERR, "could not exec '%s'", *debugger_args);

    exit(1);                    /* NOTREACHED */
}

char *core_of_prog(const char *filename)
{
    char *corefile;
    size_t len;
    len = strlen(filename);
    if ((corefile = malloc(sizeof(char) * (len + 6))) == NULL)
        err(EX_OSERR, "could not malloc corefile name");
    strncpy(corefile, filename, len);
    strncat(corefile, ".core", 5);
    return corefile;
}

void emit_help(void)
{
    fprintf(stderr, "Usage: bt [-D \"foodb -arg\"] [core-or-program-name]\n");
    exit(EX_USAGE);
}

void find_core_or_prog(char **corefilep, char **prognamep)
{
    char *tmp_corefile = NULL;
    char *tmp_corefile_prog = NULL;
    char *tmp_corefile_progtmp = NULL;
    char *tmp_progname = NULL;
    DIR *dirh;
    struct dirent *dp;
    struct stat statbuf;
    struct timespec core_mtime = { 0, 0 };
    struct timespec prog_mtime = { 0, 0 };

    if ((dirh = opendir(".")) == NULL)
        err(EX_IOERR, "could not open PWD");

    while ((dp = readdir(dirh)) != NULL) {
        // ignore dotfiles
        if (strncmp(dp->d_name, ".", 1) == 0)
            continue;

        if (stat(dp->d_name, &statbuf) < 0)
            err(EX_IOERR, "could not stat '%s'", dp->d_name);

        if (!S_ISREG(statbuf.st_mode))
            continue;

        // is it a *.core file or a program?
        if ((tmp_corefile_progtmp = prog_of_core(dp->d_name)) != NULL) {
            if (MORE_RECENT(statbuf.st_mtimespec, core_mtime)) {
                if (tmp_corefile != NULL)
                    free(tmp_corefile);
                if ((tmp_corefile = strndup(dp->d_name, dp->d_namlen)) == NULL)
                    err(EX_OSERR, "could not copy corefile name");
                if (tmp_corefile_prog != NULL)
                    free(tmp_corefile_prog);
                tmp_corefile_prog = tmp_corefile_progtmp;
                core_mtime.tv_sec = statbuf.st_mtimespec.tv_sec;
                core_mtime.tv_nsec = statbuf.st_mtimespec.tv_nsec;
            } else {
                free(tmp_corefile_progtmp);
            }
        } else if (HAS_X_BIT(statbuf.st_mode)) {
            if (MORE_RECENT(statbuf.st_mtimespec, prog_mtime)) {
                if (tmp_progname != NULL)
                    free(tmp_progname);
                if ((tmp_progname = strndup(dp->d_name, dp->d_namlen)) == NULL)
                    err(EX_OSERR, "could not copy program name");
                prog_mtime.tv_sec = statbuf.st_mtimespec.tv_sec;
                prog_mtime.tv_nsec = statbuf.st_mtimespec.tv_nsec;
            }
        }
    }
    closedir(dirh);

    // favor *.core and associated program but allow for a program otherwise
    if (tmp_corefile != NULL) {
        *corefilep = tmp_corefile;
        *prognamep = tmp_corefile_prog;
    } else if (tmp_progname != NULL) {
        *prognamep = tmp_progname;
    }
}

char **parse_debugger_args(char *progname, char *corefile)
{
    char **dargs, *string, *token, *willy;
    unsigned int cur_arg = 0;
    unsigned int max_arg = 8;

    if ((string = strdup(Flag_Debugger)) == NULL)
        err(EX_OSERR, "could not strdup debugger args");
    willy = string;

    if ((dargs = malloc(sizeof(char *) * max_arg)) == NULL)
        err(EX_OSERR, "could not malloc argument list");

    while ((token = strsep(&string, " ")) != NULL) {
        if ((dargs[cur_arg++] = strdup(token)) == NULL)
            err(EX_OSERR, "could not duplicate token");
        if (cur_arg > max_arg - 3) {
            max_arg <<= 1;
            if ((dargs = realloc(dargs, sizeof(char *) * max_arg)) == NULL)
                err(EX_OSERR, "could not realloc argument list");
        }
    }
    dargs[cur_arg++] = progname;
    if (corefile != NULL)
        dargs[cur_arg++] = corefile;
    dargs[cur_arg++] = (char *) 0;

    if ((dargs = realloc(dargs, sizeof(char *) * cur_arg)) == NULL)
        err(EX_OSERR, "could not realloc argument list");

    free(willy);

    return dargs;
}

char *prog_of_core(const char *filename)
{
    char *progname, *sp;
    long len;
    if ((sp = strrchr(filename, '.')) == NULL)
        return NULL;
    if (sp == filename)
        return NULL;
    if (strcmp(sp, ".core") != 0)
        return NULL;
    len = sp - filename + 1;
    if (len < 1)
        abort();
    if ((progname = malloc(sizeof(char) * (unsigned long) len)) == NULL)
        err(EX_OSERR, "could not malloc program name");
    strncpy(progname, filename, (size_t) len - 1);
    progname[len] = '\0';
    return progname;
}
