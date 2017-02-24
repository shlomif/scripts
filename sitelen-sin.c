/* Constructs a random word following the sound rules for the
 * importation of foreign words; see the Official Toki Pona Book (pu)
 * for details. */

// if on Linux it may be more sensible to use some not-arc4random() call
// to obtain random numbers, this ugly is to coax Centos7 to compile.
// also, you'll probably need to add -lbsd to the Makefile somewhere.
#ifdef __linux__
#define _XOPEN_SOURCE 500
typedef unsigned char u_char;
#include <bsd/stdlib.h>
#endif

#include <stdlib.h>
#include <unistd.h>

// odds of the appearance of letters (regardless of position)
const char consonants[242] =
    "jjjjjjjjjjkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkllllllllllllllllllllllllllllllllllllllllllllmmmmmmmmmmmmmmmmmmmmmmnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnpppppppppppppppppppppppppppsssssssssssssssssssssssssssssstttttttttttttttwwwwwwwwwwwwww";
unsigned int consonant_count = 242;

const char vowels[235] =
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiioooooooooooooooooooooooooooooooooooouuuuuuuuuuuuuuuuuuuuuuuuu";
unsigned int vowel_count = 235;

// NOTE buf should ideally be at least max syllable_count * 3 + 1
char buf[16];

int main(void)
{
    char consonant, vowel;
    int syllable_count = 2 + arc4random_uniform(3);
    int s, wordidx = 0;

    for (s = 0; s < syllable_count; s++) {
        vowel = vowels[arc4random_uniform(vowel_count)];
        // rule 2 - leading consonant-free syllable
        if (s == 0 && arc4random_uniform(100) < 20) {
            buf[wordidx++] = vowel;
        } else {
            while (1) {
                consonant = consonants[arc4random_uniform(consonant_count)];
                // rules 3, 4, 5 - exclude illegal pairings
                if (!((consonant == 'j' || consonant == 't') && vowel == 'i')
                    || (consonant == 'w' && (vowel == 'o' || vowel == 'u'))) {
                    break;
                }
            }
            buf[wordidx++] = consonant;
            buf[wordidx++] = vowel;
        }
        // rule 1 - optional trailing 'n' after vowel
        if (arc4random_uniform(100) < 7)
            buf[wordidx++] = 'n';
    }
    buf[wordidx++] = '\n';

    write(STDOUT_FILENO, buf, wordidx);

    return 0;
}
