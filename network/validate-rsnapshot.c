/* validate-rsnapshot - check SSH_ORIGINAL_COMMAND for rsnapshot */

#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef SYSLOG
#include <syslog.h>
#endif
#include <sysexits.h>
#include <unistd.h>

#include <pcre.h>

int Flag_Preview;               /* -n */

/* NOTE this restricts the length of the options (to prevent the remote
 * from saying -vv...) and the length of the filesystem path and greatly
 * restricts the characters that that path may contain */
/* NOTE the flags may change randomly depending on the rsync version as
 * they did between Centos 7.4 and Centos 7.5 */
#define SUBSTRINGS 7
char *pattern =
    "^(rsync) (--server) (--sender) (-[vnlHogDtprRxSe.iLsfxC]{1,24})"
    " (--numeric-ids) ([.]) ([A-Za-z0-9+_./][A-Za-z0-9+_./-]{0,1023})$";
#define MAX_CMD_LEN 1024+24+42+1

#define MAX_OFFSETS ((1+SUBSTRINGS)*3)

int main(int argc, char *argv[])
{
#ifdef SYSLOG
    char *client;
#endif
    char *command[SUBSTRINGS + 1], *soc;
    int ch;

    const char *error;
    int erroffset, i, rc;
    int offsets[MAX_OFFSETS];
    pcre *re;

#ifdef __OpenBSD__
    if (pledge("exec rpath stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

#ifdef SYSLOG
    openlog("validate-rsnapshot", LOG_NDELAY, LOG_AUTHPRIV);
    client = getenv("SSH_CLIENT");
    if (client == NULL)
        client = "unknown";
#endif

    if ((soc = getenv("SSH_ORIGINAL_COMMAND")) == NULL) {
#ifdef SYSLOG
        syslog(LOG_NOTICE, "no SSH_ORIGINAL_COMMAND set for %s", client);
#endif
        exit(2);
    }

    /* precompiling the pattern gives a 2% speed increase, slightly
     * smaller binary, and a large increase in incomprehensibility */
    if ((re = pcre_compile(pattern, 0, &error, &erroffset, NULL)) == NULL) {
#ifdef SYSLOG
        syslog(LOG_ERR, "pcre compile failed at offset %d: %s ??", erroffset,
               error);
#endif
        exit(3);
    }

    rc = pcre_exec(re, NULL, soc, strnlen(soc, MAX_CMD_LEN), 0, 0, offsets,
                   MAX_OFFSETS);
    if (rc < 1 + SUBSTRINGS) {
#ifdef SYSLOG
        syslog(LOG_NOTICE, "match failed for %s", client);
#endif
        exit(4);
    }

// TODO that there are not ../ runs in path

    if (access(soc + offsets[2 * SUBSTRINGS], R_OK | X_OK) != 0) {
#ifdef SYSLOG
        syslog(LOG_NOTICE, "not allowed to access '%s'",
               soc + offsets[2 * SUBSTRINGS]);
#endif
        exit(5);
    }

    for (i = 1; i <= SUBSTRINGS; i++) {
        soc[offsets[2 * i + 1]] = '\0';
        command[i - 1] = soc + offsets[2 * i];
    }
    command[SUBSTRINGS] = (char *) 0;

    while ((ch = getopt(argc, argv, "n")) != -1) {
        switch (ch) {
        case 'n':
            Flag_Preview = 1;
            break;
        default:
            errx(EX_USAGE, "only -n flag is supported");
        }
    }

    if (Flag_Preview) {
        puts(soc + offsets[2 * SUBSTRINGS]);
        exit(EXIT_SUCCESS);
    }
    execvp(command[0], command);

#ifdef SYSLOG
    syslog(LOG_NOTICE, "could not exec '%s' (%d)", command[0], errno);
#endif
    exit(6);
}
