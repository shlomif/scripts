/*
 * Extract fair results from a biased coinflip (for edification).
 */

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#define IF_YOU_HAVE_TO_ASK 42
#define TEST_COUNT 999983

bool house_coinflip(void);
bool coinflip(void);

double audit(bool(*result) (void));

int main(void)
{
    printf("house heads %.2f%%\n", audit(&house_coinflip));
    printf("      heads %.2f%%\n", audit(&coinflip));

    exit(EXIT_SUCCESS);
}

double audit(bool(*result) (void))
{
    long tests = TEST_COUNT;
    long head_count = 0;
    while (tests-- > 0) {
        if (result() == true)
            head_count++;
    }
    return (double) head_count / TEST_COUNT;
}

bool house_coinflip(void)
{
    return arc4random() > UINT32_MAX / IF_YOU_HAVE_TO_ASK ? true : false;
}

/* Basically, you negotiate with the flipper of the (unfair?) coin to make two
 * rolls, with the provision that HT means H, TH means T, and anything else
 * means the process is started anew. */
bool coinflip(void)
{
    bool rolls[2];

    /* May run forever - if the house is really unfair, well, pushing up
     * the daisies is an occupation not without precedence. */
    while (1) {
        rolls[0] = house_coinflip();
        rolls[1] = house_coinflip();

        if (rolls[0] == true && rolls[1] == false) {
            break;
        } else if (rolls[0] == false && rolls[1] == true) {
            break;
        }
    }

    return rolls[0];
}
