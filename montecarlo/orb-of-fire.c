/* Montecarlo simulation of the odds of being mutated by an Orb of Fire
 * in Dungeon Crawl Stone Soup (DCSS) even when the player is has
 * mutation resists. The resist is at 90%, which may seem like a high
 * value, though if one allows for multiple attacks, the odds of
 * escaping unscathed... well. That's what this code is for.
 *
 * Uses arc4random(), so systems with poor RNG support may need to use
 * GSL and seed the RNG from urandom, or the JKISS RNG, etc.
 */

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#include "aaarghs.h"

#define ATTACKS_MIN 2UL
#define ATTACKS_MAX 100UL

#define ATTACK_ODDS 0.9

// to limit nothings in output when attack number grows large
#define PROB_MIN 0.5

unsigned long Flag_Attacks;     // -a number of attacks
float Flag_Odds = NAN;          // -o odds of attack
unsigned long Flag_Trials;      // -c number of trials to run

int main(int argc, char *argv[])
{
    int ch;
    unsigned long mutates, *odds;
    float anyprob, prob;

    while ((ch = getopt(argc, argv, "a:c:o:")) != -1) {
        switch (ch) {

        case 'a':
            Flag_Attacks = flagtoul(ch, optarg, ATTACKS_MIN, ATTACKS_MAX);
            break;

        case 'c':
            Flag_Trials = flagtoul(ch, optarg, 1UL, LONG_MAX);
            break;

        case 'o':
            Flag_Odds = flagtof(ch, optarg, 0.0, 1.0);
            break;

        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (Flag_Attacks == 0)
        Flag_Attacks = ATTACKS_MIN;

    if (!isfinite(Flag_Odds))
        Flag_Odds = ATTACK_ODDS;

    if (Flag_Trials == 0)
        Flag_Trials = TRIALS;

    if ((odds = calloc(Flag_Attacks, sizeof(unsigned long))) == NULL)
        err(EX_OSERR, "could not calloc() odds table");

    for (unsigned long i = 0; i < Flag_Trials; i++) {
        mutates = 0;
        for (unsigned long a = 0; a < Flag_Attacks; a++) {
            if (arc4random() / (float) UINT32_MAX > Flag_Odds)
                mutates++;
        }
        odds[mutates]++;
    }

    printf("mutate\todds\t-t %ld -a %ld -o %.2f\n",
           Flag_Trials, Flag_Attacks, Flag_Odds);

    anyprob = 0.0;
    for (unsigned long a = 0; a < Flag_Attacks; a++) {
        prob = odds[a] / (float) Flag_Trials *100;
        if (prob > PROB_MIN)
            printf("%3lu\t%6.2f%%\n", a, prob);

        // and also the odds of at least one mutate...
        if (a > 0)
            anyprob += prob;
    }
    printf("\n >0\t%6.2f%%\n", anyprob);

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: orb-of-fire [-a attacks] [-c trials] [-o odds]\n");
    exit(EX_USAGE);
}
