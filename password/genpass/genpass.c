/* genpass - generate a password from /dev/urandom */

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#define C2CLEN 64
char code2char[C2CLEN] =
    { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'
};

char *genpass(const unsigned int len)
{
    int fd, i;
    char *pass;
#ifdef __OpenBSD__
    if (pledge("rpath stdio", NULL) == -1)
        err(1, "pledge failed");
#endif
    if ((pass = malloc(len)) == NULL)
        err(EX_OSERR, "malloc failed");
    if ((fd = open("/dev/urandom", O_RDONLY)) < 0)
        err(EX_IOERR, "open failed");
    if (read(fd, pass, len) != len)
        err(EX_IOERR, "read failed");
    close(fd);
    for (i = 0; i < len; i++)
        pass[i] = code2char[(unsigned char) pass[i] % C2CLEN];
    return pass;
}

int main(void)
{
    char *pass;
    pass = genpass(8);
    puts(pass);
    //free(pass);                 // otherwise, memory leak!
    return 0;
}
