/* glibc 2.7+ password hash generator, e.g. for the kickstart rootpw
 * line; alternatives include `pwdhash` included with 389-ds or such.
 * Should work on Centos 7 or equivalent; on RHEL6 the EPEL libsodium
 * package lacks sodium_mlock so that will need to be updated.
 *
 * Usage:
 *   yum -y groupinstall 'Development tools'
 *   yum -y install libsodium-devel man-pages
 *   make mkpwhash
 *   (echo todofixme; echo todofixme) | ./mkpwhash
 *
 * Or to be super secret and to not echo the password:
 *   stty -echo; ./mkpwhash > h; stty echo
 */

#define _XOPEN_SOURCE 700
#include <unistd.h>

#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/time.h>

#include <err.h>
#include <signal.h>
#include <sysexits.h>

#include "salt.h"

// FIXME adjust these as desired
#define MIN_PASS_LEN 8
#define MAX_PASS_LEN 128

/* For SHA512, see crypt(3). "$6$" + (up to) MAX_SALT + "$" + 86 */
#define HASH_LEN_MINUS_SALT 90

int main(void)
{
    char *hash, *password, *pp, *salt, *verify;
    size_t len;
    ssize_t plen, vlen;
    unsigned int salt_len;

    // limit shenanigans (a selinux policy to deny ptrace in advance
    // might also be handy)
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

    /* Turns out reading the password is right tricky in C, given that
     * getpass(3) is obsolete, readpassphrase(3) of OpenBSD fame
     * unavailable, and the termios stuff looks annoying.
     *
     * So, just read whatever from stdin for the time being. :/
     */
    verify = password = NULL;
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
    // no-\n-before-EOF edge case
    if (verify[vlen - 1] == '\n')
        vlen--;

    if (plen != vlen || strncmp(password, verify, plen) != 0) {
        sodium_memzero(password, plen);
        sodium_memzero(verify, vlen);
        errx(EX_DATAERR, "passwords do not match");
    }

    sodium_memzero(verify, vlen);

    /* Additional sanity tests, on the assumption that the password must
     * be typeable and thus reasonably short, and confined to a subset
     * of ASCII. In addition to the traditional gets(3), both Apple and
     * Google have had vulnerabilities from "too long" passwords. */
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
        errx(EX_OSERR, "crypt() returned NULL??");

    write(STDOUT_FILENO, hash, HASH_LEN_MINUS_SALT + salt_len);
    write(STDOUT_FILENO, "\n", 1);
    sodium_memzero(hash, HASH_LEN_MINUS_SALT + salt_len);

    exit(EXIT_SUCCESS);
}
