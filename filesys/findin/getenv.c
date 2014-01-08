#include <stdlib.h>

#include "findin.h"

static char *dir_list;
static bool got_env;

int get_next_env(char const *env)
{
    int c;

    if (!got_env) {
        if ((dir_list = getenv(env)) == NULL) {
            errx(EX_USAGE, "no such environment variable '%s'", env);
        }
        got_env = true;
    }

    c = *dir_list++;
    return c != '\0' ? c : EOF;
}
