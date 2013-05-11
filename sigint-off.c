/*
 * So this is due to the puzzler where `tail -f ...; echo foo` may or
 * may not run the `echo` after control+c is pressed in ZSH. It turns
 * out that modern shells are quite complicated in their signal and
 * job handling:
 *
 * http://www.zsh.org/mla/workers/2013/msg00454.html
 */

#include <sys/types.h>

#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <sysexits.h>

void polonius_polka(int sig);

int main()
{
    /* no signal handler */

    for (;;);

    exit(EXIT_SUCCESS);
}

void polonius_polka(int sig)
{
    errx(EX_NOUSER, "oh I am slain");   // 128+sig does not change what zsh sees
}
