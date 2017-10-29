/* Kills itself with SIGINT. This sort of thing confuses many, with
 * perhaps the most imporant point being that Control+C in a terminal
 * sends the SIGINT signal to every process in that terminal's
 * foreground process group (unless the termios mode has been fiddled
 * with, plus random interference from modern shells and their job
 * control, etc).
 *
 * See e.g. Steven's APUE for gory details.
 */

#include <signal.h>

int main(void)
{
    raise(SIGINT);
}
