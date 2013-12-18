/*
 * Imagine a vast number of villages, with paths between them, these
 * paths being used by runners to exchange messages. The villagers all
 * speak the same language--English, Klingon, whatever, the language is
 * not important. What is important is that for any message exchanged
 * between the villages, it must be written down, and must be written
 * down with the letters of the words reversed: "good day" is written
 * "doog yad", and then a runner delivers that message to the next
 * village. Therein, the translator Olef Byteswapson reads the message,
 * "doog yad", and announces to his village that the other village has
 * said "good day". They all agree that this is so, and send back the
 * reply--"dna doog yad ot uoy sa llew"--and then a runner carries this
 * reply to the original village. Therein, the counterpart to Olef,
 * another byte swapper, from a long line of byte swappers, reads the
 * message, and announces to her village the reply thus previously
 * stated. And so it goes on through the day.
 *
 * How did this most sorry state of affairs come to be? Well, back in
 * the day, "doog yad" was the actual language, as spoken between the
 * few big castles and towers of the land. There were also some little
 * villages, but they did not speak with anyone, at least not yet, and
 * spoke using the (equally arbitrary)[1] "good day" form. Now
 * eventually the towers and castles went away, or anyways became much
 * less important, while the little villages multiplied, and yet the
 * same tradition of speaking in the manner of the big castles and
 * towers carried on, at least when exchanging messages between places.
 *
 * This is how the Internet presently works.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>

#define CALLING_BYTE_ORDERS 4   /* GOLDEN_RINGS 5 etc. */

int
main(void)
{
    int i, segment;
    int order[CALLING_BYTE_ORDERS];
    struct in_addr endiant;

    /* test address - 01111111.00000000.00000001.11111111 */
    inet_pton(AF_INET, "127.0.1.255", &endiant);

    /* This call normalizes the order to 3 2 1 0 on both big-endian
     * Debian/ppc QEMU instance and little-endian host OpenBSD/amd64. */
    endiant.s_addr = htonl(endiant.s_addr);

    for (i = 0; i < CALLING_BYTE_ORDERS; i++) {
        segment = endiant.s_addr >> (i * 8) & 255;
        switch (segment) {
        case 127:
            order[i] = 0;
            break;
        case 0:
            order[i] = 1;
            break;
        case 1:
            order[i] = 2;
            break;
        case 255:
            order[i] = 3;
            break;
        default:
            abort(); abort(); abort();
        }
    }

    /* Expect 0 1 2 3 on little-endian and 3 2 1 0 on big-endian, and
     * something completely different on say the PDP-11. Unless the
     * order is normalized via htonl(). */
    printf("ordering: %d %d %d %d\n", order[0], order[1], order[2], order[3]);

    exit(0);
}

/* [1] There has actually been not a little fuss over whether words such
 * as "good day" or more properly "doog yad" are arbitrary forms, or
 * not. But we wouldn't want to bog down communication with Linguistics,
 * now would we? */
