#include <stdlib.h>

#include "findin.h"

static char *dir_list;
static int got_env = FALSE;

int get_next_env(char *env)
{
    int c;

    if (got_env != TRUE) {
        dir_list = getenv(env);
        if (dir_list == NULL) {
            fprintf(stderr, "no such environment variable: %s\n", env);
            exit_status = EX_USAGE;
            return EOF;
        }
        got_env = TRUE;
    }

    c = *dir_list++;
    return c != '\0' ? c : EOF;
}
