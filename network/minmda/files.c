#include <errno.h>

#include "minmda.h"

void gen_filenames(const char *dir, const char *hostid, char **tmp, char **new)
{
#ifndef __OpenBSD__
    int fd;
#endif
    pid_t pid;
    struct timeval now;
    uint32_t rand;

    if (gettimeofday(&now, NULL) == -1)
        err(MEXIT_STATUS, "gettimeofday failed");
    pid = getpid();

    /* in the event that the pid and time repeat for some reason ... */
#ifdef __OpenBSD__
    rand = arc4random();
#else
    if ((fd = open("/dev/urandom", O_RDONLY)) == -1)
        err(MEXIT_STATUS, "open /dev/urandom failed");
    if (read(fd, &rand, sizeof(uint32_t)) != sizeof(uint32_t))
        err(MEXIT_STATUS, "read from /dev/urandom failed");
    close(fd);
#endif

    if (asprintf
        (tmp, "%s/tmp/%ld.%d_%x_%d.%s", dir, (long) now.tv_sec,
         (int) now.tv_usec, rand, pid, hostid) < 0)
        err(MEXIT_STATUS, "asprintf failed");
    if (strnlen(*tmp, PATH_MAX) == PATH_MAX)
        err(MEXIT_STATUS, "mailbox path exceeds PATH_MAX");
    if (asprintf
        (new, "%s/new/%ld.%d_%x_%d.%s", dir, (long) now.tv_sec,
         (int) now.tv_usec, rand, pid, hostid) < 0)
        err(MEXIT_STATUS, "asprintf failed");
}

void make_paths(const char *dir)
{
    char *curdir, *tmpdir, *newdir;

    if (mkdir(dir, 0700) == -1) {
        if (errno != EEXIST)
            err(MEXIT_STATUS, "mkdir failed '%s'", dir);
    }

    /* cur dir - minmda does not use the cur dir but mutt fails to load
     * the mailbox if only {tmp,new} exist -- ensure it exists */
    if (asprintf(&curdir, "%s/cur", dir) < 0)
        err(MEXIT_STATUS, "asprintf failed");
    if (mkdir(curdir, 0700) == -1) {
        if (errno != EEXIST)
            err(MEXIT_STATUS, "mkdir failed '%s'", dir);
    }
    free(curdir);

    /* tmp dir is where new messages are created, verified, maybe
     * unlinked from */
    if (asprintf(&tmpdir, "%s/tmp", dir) < 0)
        err(MEXIT_STATUS, "asprintf failed");
    if (mkdir(tmpdir, 0700) == -1) {
        if (errno != EEXIST)
            err(MEXIT_STATUS, "mkdir failed '%s'", dir);
    }

    /* new dir the tmp-file is only renamed into */
    if (asprintf(&newdir, "%s/new", dir) < 0)
        err(MEXIT_STATUS, "asprintf failed");
    if (mkdir(newdir, 0700) == -1) {
        if (errno != EEXIST)
            err(MEXIT_STATUS, "mkdir failed '%s'", dir);
    }
#ifdef __OpenBSD__
    /* new in OpenBSD 6.4 */
    if (unveil(dir, "r") == -1)
        err(MEXIT_STATUS, "unveil failed");
    if (unveil(tmpdir, "crw") == -1)
        err(MEXIT_STATUS, "unveil failed");
    if (unveil(newdir, "c") == -1)
        err(MEXIT_STATUS, "unveil failed");
    /* no more changes to filesystem access beyond here */
    if (unveil(NULL, NULL) == -1)
        err(MEXIT_STATUS, "unveil failed");
#endif

    free(newdir);
    free(tmpdir);
}
