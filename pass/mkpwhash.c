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

#include <sys/resource.h>
#include <sys/time.h>

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sysexits.h>

/* The compile may need `pkg-config --cflags --libs libsodium` or even
 * setting PKG_CONFIG_PATH depending on how exotic a location libsodium
 * was installed to.
 */
#include <sodium.h>

#define MIN_PASS_LEN 7
#define MAX_PASS_LEN 67

#define MAX_SALT 16U            // fixed length for all present fancier hashes
#define SALT_LEN 4U + MAX_SALT

/* For SHA512, see crypt(3). "$6$" + MAX_SALT + "$" + 86. Does not include
 * the \0, though there is probably little need to memzero that.
 */
#define HASH_LEN 106U

#define SALTCHARS_LEN 64U
char Salt_Chars[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

int main(void)
{
    char *hash, *password, *pp, *verify;
    char salt[SALT_LEN + 1], *sp;
    size_t len;
    ssize_t plen, vlen;

    // limit shenanigans
    if (setrlimit(RLIMIT_CORE, &(struct rlimit) {
                  0, 0}) == -1)
        err(EX_OSERR, "could not disable coredumps");

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
    password[--plen] = '\0';    // nix the \n
    if (plen < MIN_PASS_LEN)
        errx(EX_DATAERR, "password is too short");
    sodium_mlock(password, plen);

    if ((vlen = getline(&verify, &len, stdin)) < 0)
        err(EX_IOERR, "could not read password from stdin");
    // no-\n-before-EOF edge case
    if (verify[vlen - 1] == '\n')
        vlen--;
    sodium_mlock(verify, vlen);

    if (plen != vlen || strncmp(password, verify, plen) != 0)
        errx(EX_DATAERR, "passwords do not match");

    sodium_memzero(verify, vlen);
    sodium_mlock(salt, SALT_LEN);

    /* Additional sanity tests, on the assumption that the password must
     * be typeable and thus reasonably short, and confined to a subset
     * of ASCII. In addition to the traditional gets(3), both Apple and
     * Google have had vulnerabilities from "too long" passwords. */
    if (plen > MAX_PASS_LEN)
        errx(EX_DATAERR, "password is too long");
    pp = password;
    while (*pp != '\0') {
        if (*pp < 32 || *pp > 126)
            errx(EX_DATAERR, "character in password out of range");
        pp++;
    }

    strncpy(salt, "$6$", 3);    // SHA512 indicator, again see crypt(3)
    sp = &salt[3];

    for (unsigned int i = 0; i < MAX_SALT; i++)
        *sp++ = Salt_Chars[randombytes_uniform((uint32_t) SALTCHARS_LEN)];
    *sp++ = '$';
    *sp = '\0';

    hash = crypt(password, salt);
    sodium_memzero(password, plen);
    sodium_memzero(salt, SALT_LEN);
    if (!hash)
        errx(EX_OSERR, "crypt() returned NULL??");

    write(STDOUT_FILENO, hash, HASH_LEN);
    putchar('\n');
    sodium_memzero(hash, HASH_LEN);

    exit(EXIT_SUCCESS);
}
