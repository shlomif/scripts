/* Argument handling utility routines. See also strtonum(3) on OpenBSD. */

#include "aaarghs.h"

float flagtof(const int flag, const char *flagarg,
              const float min, const float max)
{
    char *ep;
    float val;

    errno = 0;
    val = strtof(flagarg, &ep);
    if (flagarg[0] == '\0' || *ep != '\0')
        errx(EX_DATAERR, "could not parse float from -%c '%s'", flag, flagarg);
    if (errno == ERANGE)
        errx(EX_DATAERR, "value for -%c '%s' is not a float", flag, flagarg);
    if (isfinite(min) && val < min)
        errx(EX_DATAERR, "value for -%c '%s' is below min %.2f", flag, flagarg,
             min);
    if (isfinite(max) && val > max)
        errx(EX_DATAERR, "value for -%c '%s' exceeds max %.2f", flag, flagarg,
             max);
    return val;
}

unsigned long flagtoul(const int flag, const char *flagarg,
                       const unsigned long min, const unsigned long max)
{
    char *ep;
    unsigned long val;

    errno = 0;
    val = strtoul(flagarg, &ep, 10);
    if (flagarg[0] == '\0' || *ep != '\0')
        errx(EX_DATAERR, "could not parse unsigned long from -%c '%s'", flag,
             flagarg);
    if (errno == ERANGE && val == ULONG_MAX)
        errx(EX_DATAERR, "value for -%c '%s' exceeds ULONG_MAX %lu", flag,
             flagarg, ULONG_MAX);
    if (min != 0 && val < min)
        errx(EX_DATAERR, "value for -%c '%s' is below min %lu", flag, flagarg,
             min);
    if (max != ULONG_MAX && val > max)
        errx(EX_DATAERR, "value for -%c '%s' exceeds max %lu", flag, flagarg,
             max);
    return val;
}
