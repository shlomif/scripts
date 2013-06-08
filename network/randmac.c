/* Generates random MAC address. Another option would be something like:
 
#!/usr/bin/env perl
my @hex_chars = ( 0 .. 9, 'a' .. 'f' );
chomp( my $mac = shift || 'XX:XX:XX:XX:XX:XX' );
$mac =~ s/X/$hex_chars[rand @hex_chars]/eg;
print $mac, "\n";
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char hexval[] = "abcdef0123456789";

int main(int argc, char *argv[])
{
    char mac[] = "XX:XX:XX:XX:XX:XX";
    char *mp;

    srandomdev();

    if (argv[1] != NULL)
        (void) strncpy(mac, argv[1], sizeof(mac) - 1);

    mp = mac;
    while (*mp != '\0') {
        if (*mp == 'X')
            *mp = hexval[random() % (sizeof(hexval) - 1)];
        mp++;
    }

    printf("%s\n", mac);

    return 0;
}
