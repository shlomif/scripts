/* bigpass - crypto_pwhash foo (for memory usage, etc) */

#include <sys/resource.h>

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include <sodium.h>

int Return_Value = EXIT_SUCCESS;

unsigned long long Flag_Memlim = 10000; // -m
unsigned long long Flag_Oplim = 10000;  // -o

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;

    char pwhash[crypto_pwhash_scryptsalsa208sha256_STRBYTES];
    const char *password = "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdf";

    struct rusage procuse;

#ifdef __OpenBSD__
    if (pledge("stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    while ((ch = getopt(argc, argv, "h?m:o:")) != -1) {
        switch (ch) {

        case 'm':
            if (sscanf(optarg, "%llu", &Flag_Memlim) != 1) {
                errx(EX_USAGE, "could not parse -m '%s' option", optarg);
            }
            break;

        case 'o':
            if (sscanf(optarg, "%llu", &Flag_Oplim) != 1) {
                errx(EX_USAGE, "could not parse -o '%s' option", optarg);
            }
            break;

        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (crypto_pwhash_scryptsalsa208sha256_str
        (pwhash, password, strlen(password),
         Flag_Oplim, (size_t) Flag_Memlim) != 0) {
        err(EX_SOFTWARE, "crypto_pwhash_scryptsalsa208sha256_str() failed");
    }

    if (crypto_pwhash_scryptsalsa208sha256_str_verify
        (pwhash, password, strlen(password)) != 0) {
      err(1, "could not verify password??");
    }

    if (getrusage(RUSAGE_SELF, &procuse) == -1)
        err(EX_OSERR, "getrusage() failed");

    fprintf(stderr, "dbg %ld %ld %ld %ld\n", procuse.ru_maxrss,
            procuse.ru_ixrss, procuse.ru_idrss, procuse.ru_isrss);
    sleep(7);

    exit(Return_Value);
}

void emit_help(void)
{
    fputs("Usage: bigpass [-m memlimit] [-o opslimit]\n", stderr);
    exit(EX_USAGE);
}
