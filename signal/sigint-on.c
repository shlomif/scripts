/* sigint-on - so this is due to the puzzler where `tail -f ...; echo
 * foo` may or may not run the `echo` after control+c is pressed in ZSH.
 * it turns out that modern shells are quite complicated in their signal
 * and job handling:
 *
 *   http://www.zsh.org/mla/workers/2013/msg00454.html
 *   https://www.cons.org/cracauer/sigint.html
 */

#include <sys/types.h>

#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

void polonius_polka(int sig);

int main()
{
#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif
    signal(SIGINT, polonius_polka);
    for (;;);
    exit(EXIT_SUCCESS);
}

void polonius_polka(int sig)
{
    warnx("oh I am slain");
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}
