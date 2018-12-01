/* glibc 2.7+ password hash generator e.g. for the kickstart rootpw
 * line. RHEL6 or earlier will need to upgrade libsodium to a more
 * modern version of that library
 *
 * Usage:
 *   yum -y groupinstall 'Development tools'
 *   yum -y install epel-release man-pages
 *   yum -y install libsodium-devel
 *   make mkpwhash
 *   (echo Hunter2; echo Hunter2) | ./mkpwhash
 */

#define _XOPEN_SOURCE 700

#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/time.h>

#include <err.h>
#include <signal.h>
#include <sysexits.h>
#include <termios.h>
#include <unistd.h>

#include "salt.h"

// TWEAK adjust these as desired
#define MIN_PASS_LEN 7
#define MAX_PASS_LEN 128

/* for SHA512, see crypt(3)  "$6$" + (up to) MAX_SALT + "$" + 86  */
#define HASH_LEN_MINUS_SALT 90

struct termios Orig_Termios;

void restore_termios(void);

int main(void)
{
    char *hash, *password = NULL, *pp, *salt, *verify = NULL;
    size_t len;
    ssize_t plen, vlen;
    struct termios noecho;
    unsigned int salt_len;

    /* limit shenanigans (a selinux policy to deny ptrace might also
     * be handy) */
    if (prctl(PR_SET_DUMPABLE, 0) == -1)
        err(EX_OSERR, "could not disable PR_SET_DUMPABLE");

    sigset_t block;
    sigemptyset(&block);
    sigaddset(&block, SIGINT);
    sigaddset(&block, SIGTERM);
    sigaddset(&block, SIGHUP);
    sigaddset(&block, SIGPIPE);
    sigaddset(&block, SIGQUIT);
    sigaddset(&block, SIGALRM);
    sigaddset(&block, SIGTSTP);
    sigaddset(&block, SIGTTIN);
    sigaddset(&block, SIGTTOU);
    if (sigprocmask(SIG_BLOCK, &block, NULL) == -1)
        err(EX_OSERR, "sigprocmask() failed");

    /* OpenBSD meanwhile has readpassphrase(3), sigh */
    if (tcgetattr(STDIN_FILENO, &Orig_Termios) == 0) {
        atexit(restore_termios);
        noecho = Orig_Termios;
        noecho.c_lflag &= ~ECHO;
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &noecho) != 0)
            err(EX_IOERR, "tcsetattr failed");
    }

    if ((plen = getline(&password, &len, stdin)) < 0)
        err(EX_IOERR, "could not read password from stdin");
    sodium_mlock(password, MAX_PASS_LEN);
    password[--plen] = '\0';    // nix the \n
    if (plen < MIN_PASS_LEN) {
        sodium_memzero(password, plen);
        errx(EX_DATAERR, "password is too short");
    }

    if ((vlen = getline(&verify, &len, stdin)) < 0)
        err(EX_IOERR, "could not read password from stdin");
    sodium_mlock(verify, MAX_PASS_LEN);
    /* no-\n-before-EOF edge case */
    if (verify[vlen - 1] == '\n')
        vlen--;

    if (plen != vlen || strncmp(password, verify, plen) != 0) {
        sodium_memzero(password, plen);
        sodium_memzero(verify, vlen);
        errx(EX_DATAERR, "passwords do not match");
    }

    sodium_memzero(verify, vlen);

    /* additional sanity tests, on the assumption that the password must
     * be typeable and thus reasonably short, and confined to a subset
     * of ASCII. in addition to the traditional gets(3) goof both Apple
     * and Google have had vulnerabilities from "too long" passwords */
    if (plen > MAX_PASS_LEN) {
        sodium_memzero(password, plen);
        errx(EX_DATAERR, "password is too long");
    }
    pp = password;
    while (*pp != '\0') {
        if (*pp < 32 || *pp > 126) {
            sodium_memzero(password, plen);
            errx(EX_DATAERR, "character in password out of range");
        }
        pp++;
    }

    salt_len =
        MIN_SALT + randombytes_uniform((uint32_t) MAX_SALT - MIN_SALT + 1);

    if ((salt = make_salt_sha512(salt_len)) == NULL) {
        sodium_memzero(password, plen);
        err(EX_OSERR, "could not generate salt");
    }
    sodium_mlock(salt, MAX_SALT);

    hash = crypt(password, salt);
    sodium_memzero(password, plen);
    sodium_memzero(salt, salt_len);
    //free(salt);
    if (!hash)
        errx(EX_OSERR, "crypt() returned NULL ??");

    write(STDOUT_FILENO, hash, HASH_LEN_MINUS_SALT + salt_len);
    write(STDOUT_FILENO, "\n", 1);
    sodium_memzero(hash, HASH_LEN_MINUS_SALT + salt_len);

    exit(EXIT_SUCCESS);
}

void restore_termios(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &Orig_Termios);
}
