/* LD_PRELOAD library to fake open(2) calls during KickStart testing */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

typedef int (*orig_open) (const char *path, int oflag, ...);

int open(const char *path, int oflag, ...)
{
    orig_open fn;
    mode_t cmode = 0;
    va_list ap;
    if ((oflag & O_CREAT) == O_CREAT) {
        va_start(ap, oflag);
        cmode = (mode_t) va_arg(ap, int);
        va_end(ap);
    }
    /* see also the env settings made in the testks script */
    if (strncmp(path, "/proc/cmdline", 14) == 0)
        path = getenv("FO_CMDLINE");
    if (strncmp(path, "/tmp/ks-include", 16) == 0)
        path = getenv("FO_KSINC");
    fn = (orig_open) dlsym(RTLD_NEXT, "open");
    return fn(path, oflag, cmode);
}
