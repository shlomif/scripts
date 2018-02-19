/* validate-rsnapshot - check SSH_ORIGINAL_COMMAND for rsnapshot */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#ifdef SYSLOG
#include <syslog.h>
#endif
#include <unistd.h>

#include <pcre.h>

/* NOTE this restricts the length of the options (to prevent the remote
 * from saying -vv...) and the length of the filesystem path and greatly
 * restricts the characters that that path may contain */
#define SUBSTRINGS 7
char *pattern =
    "^(rsync) (--server) (--sender) (-[vnlHogDtprRxSe.iLsfxC]{1,24})"
    " (--numeric-ids) ([.]) (/[A-Za-z0-9+_./-]{1,1024})$";
#define MAX_CMD_LEN 1024+24+42+1

#define MAX_OFFSETS ((1+SUBSTRINGS)*3)

int main(void)
{
    char *client = "unknown";
    char *command[SUBSTRINGS + 1], *soc;

    const char *error;
    int erroffset, i, rc;
    int offsets[MAX_OFFSETS];
    pcre *re;

#ifdef __OpenBSD__
    if (pledge("exec rpath stdio", NULL) == -1)
        return 1;
#endif

#ifdef SYSLOG
    openlog("validate-rsnapshot", LOG_NDELAY, LOG_AUTHPRIV);
    client = getenv("SSH_CLIENT");
#endif

    if ((soc = getenv("SSH_ORIGINAL_COMMAND")) == NULL) {
#ifdef SYSLOG
        syslog(LOG_NOTICE, "no SSH_ORIGINAL_COMMAND set for %s", client);
#endif
        return 1;
    }

    /* precompiling the pattern gives a 2% speed increase, slightly
     * smaller binary, and a large increase in incomprehensibility */
    if ((re = pcre_compile(pattern, 0, &error, &erroffset, NULL)) == NULL) {
#ifdef SYSLOG
        syslog(LOG_ERR, "pcre compile failed at offset %d: %s ??", erroffset,
               error);
#endif
        return 1;
    }

    rc = pcre_exec(re, NULL, soc, strnlen(soc, MAX_CMD_LEN), 0, 0, offsets,
                   MAX_OFFSETS);
    if (rc < 1 + SUBSTRINGS) {
#ifdef SYSLOG
        syslog(LOG_NOTICE, "match failed for %s", client);
#endif
        return 1;
    }

    if (access(soc + offsets[2 * SUBSTRINGS], R_OK | X_OK) != 0) {
#ifdef SYSLOG
        syslog(LOG_NOTICE, "not allowed to access '%s'",
               soc + offsets[2 * SUBSTRINGS]);
#endif
        return 1;
    }

    for (i = 1; i <= SUBSTRINGS; i++) {
        soc[offsets[2 * i + 1]] = '\0';
        command[i - 1] = soc + offsets[2 * i];
    }
    command[SUBSTRINGS] = (char *) 0;

    execvp(command[0], command);

#ifdef SYSLOG
    syslog(LOG_NOTICE, "could not exec '%s' (%d)", command[0], errno);
#endif
    return 1;
}
