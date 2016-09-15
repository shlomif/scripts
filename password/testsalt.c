/* Allows for such testing as
 *   ./testsalt | r-fu equichisq -
 */

#include <unistd.h>

#include "salt.h"

// contiguous range of positive integers for the allowed salt characters
// as dunno if chisq or other tests okay with the discontinuous number
// ranges the salt characters have in ASCII
const char Char_Map[] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
    5, 6, 7, 8, 9,
    10, 11, 12, 0, 0, 0, 0, 0, 0, 0, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26,
    27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 0, 0, 0, 0, 0, 0, 39, 40,
    41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64
};

int main(void)
{
    char *salt, *sp;
    unsigned int salt_len;

    for (unsigned int i = 0; i < 100000; i++) {
        salt_len =
            MIN_SALT + randombytes_uniform((uint32_t) MAX_SALT - MIN_SALT + 1);

        // NOTE salt includes the leading $6$ and trailing $ required by
        // crypt(3) so must strip that off to check that the salt is
        // properly random.
        salt = make_salt_sha512(salt_len);
        if (!salt) abort();
        sp = salt + 3;

        //write(STDOUT_FILENO, sp, salt_len);
        //write(STDOUT_FILENO, "\n", 1);
        // but instead need numbers to feed e.g. R chisq.test
        while (salt_len-- > 0) {
            printf("%d\n", Char_Map[(int) *sp++]);
        }

        free(salt);             // there is no such thing as free salt
    }

    exit(EXIT_SUCCESS);
}
