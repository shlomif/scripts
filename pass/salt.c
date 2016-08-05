/* Funny story: the salt generation code of shadow-utils on Linux is (as
 * of August 2016) biased towards / due to buggy code that tries to
 * select 36 bits worth of entropy from a 31 bit value. Hopefully this
 * code is less buggy and more testable.
 */

#include "salt.h"

#define SALTCHARS_LEN 64U
char Salt_Chars[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

char *make_salt_sha512(unsigned int len)
{
    char *salt, *sp;

    if (len > MAX_SALT || len < MIN_SALT)
        abort();

    if ((salt = calloc(len + 5, sizeof(char))) == NULL)
        return salt;

    strncpy(salt, "$6$", 3);
    sp = &salt[3];

    for (unsigned int i = 0; i < len; i++)
        *sp++ = Salt_Chars[randombytes_uniform((uint32_t) SALTCHARS_LEN)];
    *sp++ = '$';
    *sp = '\0';

    return salt;
}
