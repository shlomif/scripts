/* Simple illustration of how differences in overall birth and death rates
 * change a population over time. */

#include <stdio.h>

int main(void)
{
    double population = 70000;
    double birth_rate = 0.010;
    double death_rate = 0.015;
    unsigned int year = 2014;

    double orig_pop = population;

    printf("population in %d is %.0f\n", year, population);
    for (; year < 2114; year++) {
        population += population * birth_rate - population * death_rate;
    }
    printf("population in %d is %.0f (%.2f%% with %.2f%% delta)\n", year,
           population, (population - orig_pop) / orig_pop * 100,
           (birth_rate - death_rate) * 100);

    return 0;
}
