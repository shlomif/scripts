#include "findin.h"

static bool done_reading;
static bool got_env;
static char *envp;

int get_next_env(const char *env)
{
    int c;

    if (!got_env) {
        if ((envp = getenv(env)) == NULL) {
            errx(EX_USAGE, "no such environment variable '%s'", env);
        }
        got_env = true;
    }

    if (!done_reading) {
        c = *envp++;
        if (c == '\0')
            done_reading = true;
    }

    return done_reading ? EOF : c;
}
