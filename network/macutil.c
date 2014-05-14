#include "macutil.h"

/* NOTE arc4random() probably requires libbsd and -lbsd to compile on Linux,
 * though 'cc -lbsd -std=c99 ...' with libbsd from EPEL blows up on RHEL6,
 * so meh... */

/* Converts a string "00:01:02:XX:XX:XX" into a hopefully appropriately
 * sized array of uint8_t (unsigned char) values. */
int
str2mac(const char *str, uint8_t * mac, size_t mac_size)
{
    bool is_lsn = false;
    bool is_noise = false;
    char c;
    size_t i = 0;
    unsigned long val;

    if (!str || !mac || mac_size == 0)
	abort();		/* something awry with input, bail out */

    /* NOTE this method does nothing if str contains more than expected
     * amount of input; any extra material in str is ignored. */
    while ((c = *str++) != '\0' && i < mac_size) {
	if (isxdigit(c)) {;
	    errno = 0;
	    val = strtoul(&c, NULL, BASE_HEX);
	    /* should not happen. probably not portable. */
	    if ((val == 0 && errno == EINVAL) || val > 0xF)
		abort();

	    if (is_lsn) {
		*(mac + i) = (uint8_t) ((uint8_t) val | *(mac + i) << NIBBLE);
		i++;
		is_lsn = false;
	    } else {
		*(mac + i) = (uint8_t) val;
		is_lsn = true;
	    }
	} else if (c == 'X') {
	    if (is_lsn) {
		*(mac + i) = (uint8_t) ((arc4random() & NIBBLE_MASK) | *(mac + i) << NIBBLE);
		i++;
		is_lsn = false;
	    } else {
		*(mac + i) = arc4random() & NIBBLE_MASK;
		is_lsn = true;
	    }
	} else {
	    /* non-hex-or-X bumps to new slot (but only once) */
	    if (is_lsn && !is_noise)
		i++;
	    is_noise = true;
	    continue;
	}
	is_noise = false;
    }

    if (i < mac_size)
	return -1;		/* underflow, string too short for MAC */

    return 0;
}
