/* justblocks - it just blocks */

#include <err.h>
#include <unistd.h>

char buf[1024];

int main(void)
{
    int fds[2];
    if (pipe(fds) != 0) err(1, "pipe failed ??");
    while (1) read(fds[0], buf, 1024);
    return 1;
}
