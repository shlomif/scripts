/* logsoc - logs the SSH_ORIGINAL_COMMAND
 *
 * Usage: in authorized_keys set something like
 *   command="/path/to/logsoc" ssh-...
 *   command="/path/to/logsoc /some/other/command" ssh-...
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

/* limit length per RFC 3164 section 6.1 (with space for other things) */
#define SYSLOG_MESSAGE_MAX 768

int main(int argc, char *argv[])
{
    char *client, *msg, *soc;
    size_t i, len;

#ifdef __OpenBSD__
    if (pledge("exec stdio", NULL) == -1)
        return 1;
#endif

    openlog("logsoc", LOG_NDELAY, LOG_AUTHPRIV);

    client = getenv("SSH_CLIENT");
    if (client == NULL) {
        client = "unknown";
    } else {
        soc = strrchr(client, ' ');
        if (soc != NULL)
            *soc = '\0';
        soc = strchr(client, ' ');
        if (soc != NULL)
            *soc = ':';
    }

    if ((soc = getenv("SSH_ORIGINAL_COMMAND")) == NULL) {
        syslog(LOG_NOTICE, "client %s no command", client);
        return 1;
    }

    len = strnlen(soc, SYSLOG_MESSAGE_MAX);
    msg = strndup(soc, len);
    if (msg == NULL) {
        syslog(LOG_ERR, "error strndup failed (%d)", errno);
        return 1;
    }
    /* limit content per RFC 3164 section 4.1.3 */
    for (i = 0; i < len; i++) {
        if (msg[i] < 32 || msg[i] > 126)
            msg[i] = '.';
    }
    syslog(LOG_NOTICE, "client %s command \"%s\"", client, msg);
    //free(msg);

    if (argc < 2)
        return 1;

    argv++;
    execvp(*argv, argv);

    syslog(LOG_ERR, "error exec failed (%d)", errno);
    return 1;
}
