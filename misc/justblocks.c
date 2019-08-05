/* justblocks - it just blocks */

#include <err.h>
#include <unistd.h>

char buf[1024];

int main(void)
{
    int fds[2];
#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif
    if (pipe(fds) != 0) err(1, "pipe failed ??");
    while (1) read(fds[0], buf, 1024);
    return 1;
}
