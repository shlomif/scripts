/* minmda - a minimal mail delivery agent */

#if defined(linux) || defined(__linux) || defined(__linux__)
#define _GNU_SOURCE
#endif

#include <locale.h>
#include <signal.h>

/* libsodium for checksums */
#include <sodium.h>

#include "minmda.h"

void write_buf(const char *mfile_tmp, int mailfd,
               crypto_hash_sha256_state * hash_state, const unsigned char *buf,
               ssize_t amount);

unsigned char buf[MBUF_MAX];
char *mfile_tmp = NULL;

int main(int argc, char *argv[])
{
    /* arguments */
    char *cp, *maildir_path, *hostid;
    size_t len;
    unsigned char *bp;

    /* output file foo */
    char *mfile_new = NULL;
    int mailfd, seen_something = 0;
    ssize_t amount;

    /* checksums via libsodium */
    unsigned char hash_before[crypto_hash_sha256_BYTES];
    unsigned char hash_after[crypto_hash_sha256_BYTES];
    crypto_hash_sha256_state hash_state;

#ifdef __OpenBSD__
    if (pledge("stdio rpath wpath cpath", NULL) == -1)
        err(MEXIT_STATUS, "pledge failed");
#endif

    /* thought here is to signal a temporary failure on various common
     * signals; alternatives would be to not handle signals (e.g. if the
     * caller for any non-zero exit does what you want) or to block
     * signals once have the message and try to get it to disk (better
     * at obtaining mail, at cost of possible duplicates if the caller
     * has gone away during that blocked rename of the now delivered but
     * not acknowledged message) */
    signal(SIGTERM, tempexit);
    signal(SIGINT, tempexit);
    signal(SIGHUP, tempexit);
    signal(SIGUSR1, tempexit);
    signal(SIGUSR2, tempexit);

    setlocale(LC_ALL, "C");

    argc--;
    argv++;
    if (argc != 2) {
        fprintf(stderr, "Usage: minmda mailbox-dir host-id\n");
        exit(MEXIT_STATUS);
    }
    maildir_path = *argv++;
    len = strnlen(maildir_path, PATH_MAX);
    if (len == 0 || len >= PATH_MAX)
        errx(MEXIT_STATUS, "invalid mailbox-dir");

    cp = hostid = *argv;
    while (*cp != '\0') {
        /* "replace / with \057 and : with \072" is how the maildir docs
         * indicate how to handle "invalid host names" this here instead
         * deals only with what is illegal in a unix filename */
        if (*cp == '/')
            *cp = '_';
        cp++;
    }
    if (cp - hostid <= 0)
        errx(MEXIT_STATUS, "invalid empty host-id");

    make_paths(maildir_path);
    gen_filenames(maildir_path, hostid, &mfile_tmp, &mfile_new);

    if ((mailfd = open(mfile_tmp, O_RDWR | O_CREAT | O_EXCL, 0600)) == -1)
        err(MEXIT_STATUS, "could not open '%s'", mfile_tmp);

    crypto_hash_sha256_init(&hash_state);

    while ((amount = read(STDIN_FILENO, buf, MBUF_MAX)) > 0) {
        /* maildrop docs indicate that something emits a leading blank
         * line, and that maildrop skips that blank line. cargocult that
         * skip. actually, strip any number of leading blank lines... */
        if (!seen_something) {
            bp = buf;
            while (*bp == '\n' && amount > 0) {
                bp++;
                amount--;
            }
            if (amount > 0) {
                write_buf(mfile_tmp, mailfd, &hash_state, bp, amount);
                seen_something = 1;
            }
            continue;
        }
        write_buf(mfile_tmp, mailfd, &hash_state, buf, amount);
    }
    if (amount == -1) {         /* EOF is 0 */
        unlink(mfile_tmp);
        err(MEXIT_STATUS, "read stdin failed");
    }
    if (!seen_something) {
        unlink(mfile_tmp);
        errx(MEXIT_STATUS, "read no data from stdin");
    }
    /* beyond here, a different design might try to keep the temporary
     * delivery around even if things go awry; the present code takes
     * an all-or-nothing approach and always tries to unlink the tmp
     * file on error */
    if (fsync(mailfd) == -1) {
        unlink(mfile_tmp);
        err(MEXIT_STATUS, "fsync failed");
    }

    /* verify what was (presumably, maybe) written to disk */
    lseek(mailfd, 0, 0);

    crypto_hash_sha256_final(&hash_state, hash_before);
    crypto_hash_sha256_init(&hash_state);

#ifdef MINMDA_ALWAYS_CORRUPTS
    warnx("corrupting the output file");
    write(mailfd, "\n", 1);
#endif

    while ((amount = read(mailfd, buf, MBUF_MAX)) > 0)
        crypto_hash_sha256_update(&hash_state, buf, amount);
    if (amount == -1) {
        unlink(mfile_tmp);
        err(MEXIT_STATUS, "re-read of maildir file failed");
    }
    if (close(mailfd) == -1) {
        unlink(mfile_tmp);
        err(MEXIT_STATUS, "close failed on maildir file");
    }

    crypto_hash_sha256_final(&hash_state, hash_after);

    if (memcmp(hash_before, hash_after, crypto_hash_sha256_BYTES) != 0) {
        unlink(mfile_tmp);
        errx(MEXIT_STATUS, "failed to verify written data");
    }

#ifdef MINMDA_ALWAYS_CORRUPTS
    warnx("corruption did not happen ??");
    _exit(MEXIT_STATUS);
#endif

    if (rename(mfile_tmp, mfile_new) == -1) {
        unlink(mfile_tmp);
        errx(MEXIT_STATUS, "rename failed");
    }

    _exit(EXIT_SUCCESS);
}

void tempexit(int unused)
{
    if (mfile_tmp != NULL)
        unlink(mfile_tmp);
    _exit(MEXIT_STATUS);
}

inline void write_buf(const char *mfile_tmp, int mailfd,
                      crypto_hash_sha256_state * hash_state,
                      const unsigned char *buf, ssize_t amount)
{
    crypto_hash_sha256_update(hash_state, buf, amount);
    if (write(mailfd, buf, amount) == -1) {
        unlink(mfile_tmp);
        err(MEXIT_STATUS, "write failed");
    }
}
