#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "utilities.h"
#include "simulation.h"

#define non_expected 0
#define expected 1
#define already_read 2

void initialize_simulation(simulation *scenario, unsigned int seed)
{
    scenario->verbose = false;
    scenario->duration = 0.5;
    scenario->lethality = 0.5;
    scenario->infectivity = 0.5;
    scenario->vaccine_modifier = 1.2;
    scenario->max_steps = INT_MAX;
    scenario->seed = seed;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "%s\n", "not enough argument");
        return 1;
    }
    unsigned int seed = time(NULL);
    simulation *scenario = malloc(sizeof(simulation));
    if (scenario == NULL) {
        fprintf(stderr, "%s\n", "cannot allocate memory");
        return 1;
    }
    initialize_simulation(scenario, seed);


    int expect_duration = 0;
    int expect_lethality = 0;
    int expect_infectivity = 0;
    int expect_vaccine_modifier = 0;
    int expect_max_steps = 0;
    int expect_seed = 0;
    double converted_float;
    int converted_int;
    for (int i = 1; i < argc - 2; i++) {
        if (expect_duration == expected) {
            if ((converted_float = load_double(argv[i])) < 0) {
                free(scenario);
                return 1;
            }
            if (converted_float > 1.0 || converted_float < 0.0) {
                free(scenario);
                return 1;
            }
            scenario->duration = converted_float;
            expect_duration = already_read;
        } else if (expect_lethality == expected) {
            if ((converted_float = load_double(argv[i])) < 0) {
                free(scenario);
                return 1;
            }
            if (converted_float > 1.0 || converted_float < 0.0) {
                free(scenario);
                return 1;
            }
            scenario->lethality = converted_float;
            expect_lethality = already_read;
        } else if (expect_infectivity == expected) {
            if ((converted_float = load_double(argv[i])) < 0) {
                free(scenario);
                return 1;
            }
            if (converted_float > 1.0 || converted_float < 0.0) {
                free(scenario);
                return 1;
            }
            scenario->infectivity = converted_float;
            expect_infectivity = already_read;
        } else if (expect_vaccine_modifier == expected) {
            if ((converted_float = load_double(argv[i])) < 0) {
                free(scenario);
                return 1;
            }
            scenario->vaccine_modifier = converted_float;
            expect_vaccine_modifier = already_read;
        } else if (expect_max_steps == expected) {
            if ((converted_int = load_int(argv[i])) < 0) {
                free(scenario);
                return 1;
            }
            scenario->max_steps = converted_int;
            expect_max_steps = already_read;
        } else if (expect_seed == expected) {
            if ((converted_int = load_int(argv[i])) < 0) {
                free(scenario);
                return 1;
            }
            scenario->seed = converted_int;
            expect_seed = already_read;
        } else if (strcmp(argv[i], "--lethality") == non_expected) {
            if (expect_lethality == already_read) {
                free(scenario);
                fprintf(stderr, "%s\n", "lethality is set twice");
                return 1;
            }
            expect_lethality = expected;
        } else if (strcmp(argv[i], "--infectivity") == non_expected) {
            if (expect_infectivity == already_read) {
                free(scenario);
                fprintf(stderr, "%s\n", "infectivity is set twice");
                return 1;
            }
            expect_infectivity = expected;
        } else if (strcmp(argv[i], "--duration") == non_expected) {
            if (expect_duration == already_read) {
                free(scenario);
                fprintf(stderr, "%s\n", "duration is set twice");
                return 1;
            }
            expect_duration = expected;
        } else if (strcmp(argv[i], "--vaccine-modifier") == non_expected) {
            if (expect_vaccine_modifier == already_read) {
                free(scenario);
                fprintf(stderr, "%s\n", "vaccine-modifier is set twice");
                return 1;
            }
            expect_vaccine_modifier = expected;
        } else if (strcmp(argv[i], "--max-steps") == non_expected) {
            if (expect_max_steps == already_read) {
                free(scenario);
                fprintf(stderr, "%s\n", "max-steps is set twice");
                return 1;
            }
            expect_max_steps = expected;
        } else if (strcmp(argv[i], "--random-seed") == non_expected) {
            if (expect_seed == already_read) {
                free(scenario);
                fprintf(stderr, "%s\n", "seed is set twice");
                return 1;
            }
            expect_seed = expected;
        } else if (strcmp(argv[i], "--verbose") == non_expected) {
            scenario->verbose = expected;
        } else {
            free(scenario);
            fprintf(stderr, "%s\n", "unexpected argument");
            return 1;
        }
    }
    FILE *agents = fopen(argv[argc - 2], "r");
    if (agents == NULL) {
        fprintf(stderr, "%s\n", "cant open agents file");
        free(scenario);
        return 1;
    }
    FILE *worlds = fopen(argv[argc - 1], "r");
    if (worlds == NULL) {
        fclose(agents);
        fprintf(stderr, "%s\n", "cant open worlds file");
        free(scenario);
        return 1;
    }
    if (load_simulation(scenario, agents, worlds) != 0) {
        fclose(agents);
        fclose(worlds);
        return 1;
    }
    srand(scenario->seed);
    if (run_simulation(scenario) != 0) {
        fclose(agents);
        fclose(worlds);
        return 1;
    }
    fclose(agents);
    fclose(worlds);
    return 0;
}
