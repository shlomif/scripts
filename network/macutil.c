#include <sys/types.h>

#include <fcntl.h>

#include "macutil.h"

#ifdef __OpenBSD__
#define somerandom arc4random
#else
int rndfd;
uint32_t somerandom(void)
{
    uint32_t result;
    if (rndfd == 0) {
        if ((rndfd = open("/dev/urandom", O_RDONLY)) == -1)
            err(EX_IOERR, "open /dev/urandom failed");
    }
    if (read(rndfd, &result, sizeof(uint32_t)) != sizeof(uint32_t))
        err(EX_IOERR, "read from /dev/urandom failed");
    return result;
}
// TODO close rndfd on systems that need that
#endif

/* converts a string "00:01:02:XX:XX:XX" into a hopefully appropriately
 * sized array of uint8_t (unsigned char) values */
int str2mac(const char *str, uint8_t * mac, size_t mac_size)
{
    bool is_lsn = false;
    bool is_separator = false;
    char c;
    size_t i = 0;
    uint8_t val;

    if (!str || !mac || mac_size == 0)
        abort();                /* something awry with input, bail out */

    /* NOTE this method does nothing if str contains more than expected;
     * any extra material in str is ignored */
    while ((c = *str++) != '\0' && i < mac_size) {
        if (isxdigit(c)) {
            if (isdigit(c))
                val = c - '0';
            else if (c >= 'A' && c <= 'F')
                val = c - 'A' + 10;
            else if (c >= 'a' && c <= 'f')
                val = c - 'a' + 10;
            if (is_lsn) {
                *(mac + i) = (val | *(mac + i) << NIBBLE);
                i++;
                is_lsn = false;
            } else {
                *(mac + i) = val;
                is_lsn = true;
            }
        } else if (c == 'X') {
            if (is_lsn) {
                *(mac + i) =
                    (uint8_t) ((somerandom() & NIBBLE_MASK) | *(mac + i) <<
                               NIBBLE);
                i++;
                is_lsn = false;
            } else {
                *(mac + i) = somerandom() & NIBBLE_MASK;
                is_lsn = true;
            }
        } else {
            /* non-hex-or-X bumps to new slot (but only once) */
            if (is_lsn && !is_separator)
                i++;
            is_separator = true;
            continue;
        }
        is_separator = false;
    }

    if (i < mac_size)
        return -1;              /* underflow, string too short for MAC */

    return 0;
}
