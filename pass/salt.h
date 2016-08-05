#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The compile may need `pkg-config --cflags --libs libsodium` or even
 * setting PKG_CONFIG_PATH depending on how exotic a location libsodium
 * was installed to.
 */
#include <sodium.h>

/* Smaller salt lengths make it easier for attackers should the password
 * in question use one of the shorter lengths, and there are diminishing
 * returns for what those smaller salt lengths add to the overall
 * possibilities. Thus, aim for the longest lengths possible.
 */
#define MIN_SALT 13U
#define MAX_SALT 16U            // for all fancier hashes per crypt(3)

char *make_salt_sha512(unsigned int len);
