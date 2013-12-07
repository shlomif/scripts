#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#define BASE_HEX 0x10

#define NIBBLE_MASK 0xF
#define NIBBLE 0x4

int str2mac(const char *str, uint8_t *mac, size_t mac_size);
