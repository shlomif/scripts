#if defined(linux) || defined(__linux) || defined(__linux__)
#define _GNU_SOURCE
#endif

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
        (tmp, "%s/tmp/%ld.%d%x%d.%s", dir, (long) now.tv_sec, (int) now.tv_usec,
         rand, pid, hostid) < 0)
        err(MEXIT_STATUS, "asprintf failed");
    if (strnlen(*tmp, PATH_MAX) == PATH_MAX)
        err(MEXIT_STATUS, "mailbox path exceeds PATH_MAX");
    if (asprintf
        (new, "%s/new/%ld.%d%x%d.%s", dir, (long) now.tv_sec, (int) now.tv_usec,
         rand, pid, hostid) < 0)
        err(MEXIT_STATUS, "asprintf failed");
}

void make_paths(const char *dir)
{
    char *subdir;
    if (mkdir(dir, 0700) == -1) {
        if (errno != EEXIST)
            err(MEXIT_STATUS, "mkdir failed '%s'", dir);
    }
    if (asprintf(&subdir, "%s/tmp", dir) < 0)
        err(MEXIT_STATUS, "asprintf failed");
    if (mkdir(subdir, 0700) == -1) {
        if (errno != EEXIST)
            err(MEXIT_STATUS, "mkdir failed '%s'", dir);
    }
    if (asprintf(&subdir, "%s/new", dir) < 0)
        err(MEXIT_STATUS, "asprintf failed");
    if (mkdir(subdir, 0700) == -1) {
        if (errno != EEXIST)
            err(MEXIT_STATUS, "mkdir failed '%s'", dir);
    }
    /* minmda does not use the cur dir but mutt fails to load the
     * mailbox if only {tmp,new} exist */
    if (asprintf(&subdir, "%s/cur", dir) < 0)
        err(MEXIT_STATUS, "asprintf failed");
    if (mkdir(subdir, 0700) == -1) {
        if (errno != EEXIST)
            err(MEXIT_STATUS, "mkdir failed '%s'", dir);
    }
}
