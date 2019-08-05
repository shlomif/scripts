/* slay-self - kills itself with SIGINT. this sort of thing confuses
 * many, with perhaps the most imporant point being that Control+C in a
 * terminal sends the SIGINT signal to every process in that terminal's
 * foreground process group (unless the termios mode has been fiddled
 * with, plus random interference from modern shells and their job
 * control, etc)
 *
 * see e.g. Steven's APUE for gory details
 */

#include <err.h>
#include <signal.h>
#include <unistd.h>

int main(void)
{
#ifdef __OpenBSD__
    if (pledge("", NULL) == -1)
        err(1, "pledge failed");
#endif
    raise(SIGINT);
}
