/* Pianoteq wrapper to play the most recent *.midi file or a given
 * *.midi file or to passthrough --options directly. */

#include <sys/stat.h>

#include <dirent.h>
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define MORE_RECENT(t,v) \
    (t.tv_sec > v.tv_sec || (t.tv_sec == v.tv_sec && t.tv_nsec > v.tv_nsec))

char *pianoteq_path =
    "/Applications/Pianoteq 5/Pianoteq 5.app/Contents/MacOS/Pianoteq 5";

char **args_for_file(char *path);
char *latest_midi(void);

int main(int argc, char *argv[])
{
    char **pianoteq_args;
    char **ap = argv;
    ap++;
    if (*ap == NULL) {
        pianoteq_args = args_for_file(latest_midi());
    } else {
        /* all Pianoteq options begin with -- so if see that assume the
         * caller knows how to construct proper arguments to Pianoteq */
        if (strncmp(*ap, "--", 2) == 0) {
            pianoteq_args = argv;
        } else {
            /* assume a filename was given */
            if (*ap[0] == '\0') {
                fprintf(stderr, "Usage: pianoteq [midifile]\n");
                exit(1);
            }
            pianoteq_args = args_for_file(*ap);
        }
    }

    *pianoteq_args = pianoteq_path;

    execv(pianoteq_path, pianoteq_args);
    err(1, "exec failed");
    exit(1);                    /* NOTREACHED */
}

char **args_for_file(char *path)
{
    char **pianoteq_args;
    if ((pianoteq_args = calloc(6, sizeof(char **))) == NULL)
        err(1, "malloc failed");
    pianoteq_args[1] = "--midi";
    pianoteq_args[2] = path;
    pianoteq_args[3] = "--headless";
    pianoteq_args[4] = "--play-and-quit";
    pianoteq_args[5] = (char *) NULL;
    return pianoteq_args;
}

char *latest_midi(void)
{
    char *filename = NULL, *sp;
    DIR *dirh;
    struct dirent *dp;
    struct stat statbuf;
    struct timespec most_recent = { 0, 0 };

    if ((dirh = opendir(".")) == NULL)
        err(EX_IOERR, "could not open PWD");

    while ((dp = readdir(dirh)) != NULL) {
        if (strncmp(dp->d_name, ".", 1) == 0)
            continue;

        if ((sp = strrchr(dp->d_name, '.')) == NULL)
            continue;
        sp++;
        if (strncmp(sp, "midi", 5) != 0)
            continue;

        if (stat(dp->d_name, &statbuf) < 0)
            err(EX_IOERR, "could not stat '%s'", dp->d_name);

        if (MORE_RECENT(statbuf.st_mtimespec, most_recent)) {
            if (filename != NULL)
                free(filename);
            if ((filename = strndup(dp->d_name, dp->d_namlen)) == NULL)
                err(EX_OSERR, "could not duplicate filename");
            most_recent.tv_sec = statbuf.st_mtimespec.tv_sec;
            most_recent.tv_nsec = statbuf.st_mtimespec.tv_nsec;
        }
    }
    closedir(dirh);

    if (filename == NULL) {
        warnx("no *.midi file found");
        fprintf(stderr, "Usage: pianoteq [midifile]\n");
        exit(1);
    }
    return filename;
}
