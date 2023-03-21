//
// Created by Petr Valik on 20.06.2022.
//

#include <string.h>
#include "simulation.h"
#include "agents_parser.h"
#include "world_parser.h"

int agents_to_list(simulation *scenario)
{
    agent **agent_list = malloc(sizeof(agent *) * scenario->agents_list->agent_tail->id + 8);
    if (agent_list == NULL) {
        fprintf(stderr, "%s\n", "cannot allocate memory");
        return 1;
    }
    memset(agent_list, 0, sizeof(agent *) * scenario->agents_list->agent_tail->id + 8);
    agent *actual = scenario->agents_list->agent_head;
    while (actual != NULL) {
        agent_list[actual->id] = actual;
        actual = actual->next;
    }

    scenario->agents = agent_list;
    return 0;
}

int locations_to_list(simulation *scenario)
{
    location **locations_list = malloc(sizeof(location *) * scenario->worlds_list->world_tail->id + 8);
    if (locations_list == NULL) {
        fprintf(stderr, "%s\n", "cannot allocate memory");
        return 1;
    }
    memset(locations_list, 0, sizeof(location *) * scenario->worlds_list->world_tail->id + 8);
    location *actual = scenario->worlds_list->world_head;
    while (actual != NULL) {
        locations_list[actual->id] = actual;
        actual = actual->next;
    }

    scenario->worlds = locations_list;
    return 0;
}

void free_simulalion(simulation *scenario)
{
    free_agents(scenario->agents_list);
    free_world(scenario->worlds_list);
    free(scenario->agents);
    free(scenario->worlds);
    free(scenario);
}

void pop_agent(simulation *scenario, agent *death_agent)
{
    if (death_agent == scenario->agents_list->agent_head && death_agent == scenario->agents_list->agent_tail) {
        scenario->agents_list->agent_head = NULL;
        scenario->agents_list->agent_tail = NULL;
    } else if (death_agent == scenario->agents_list->agent_head) {
        death_agent->next->prev = NULL;
        scenario->agents_list->agent_head = death_agent->next;
    } else if (death_agent == scenario->agents_list->agent_tail) {
        death_agent->prev->next = NULL;
        scenario->agents_list->agent_tail = death_agent->prev;
    } else {
        death_agent->next->prev = death_agent->prev;
        death_agent->prev->next = death_agent->next;
    }
    free(death_agent->locations);
    free(death_agent);
}

int move_agents_on_location(simulation *scenario)
{
    agent *actual = scenario->agents_list->agent_head;
    while (actual != NULL) {
        if (actual->is_death == true) {
            agent *next = actual->next;
            pop_agent(scenario, actual);
            actual = next;
        } else {
            location *agent_location = scenario->worlds[actual->locations[actual->curent_location]];
            if (agent_location == NULL) {
                fprintf(stderr, "%s\n", "non existing location");
                return 1;
            }
            agent_location->presented_agents_number += 1;
            agent **new_array = realloc(agent_location->presented_agents, sizeof(agent *) * agent_location->presented_agents_number);
            if (new_array == NULL) {
                fprintf(stderr, "%s\n", "cannot allocate memory");
                return 1;
            }
            agent_location->presented_agents = new_array;

            new_array[agent_location->presented_agents_number - 1] = actual;

            actual->curent_location += 1;
            if (actual->curent_location >= actual->number_of_locations) {
                actual->curent_location = 0;
            }
            actual = actual->next;
        }
    }
    return 0;
}

void progress_of_disease(simulation *scenario)
{
    location *actual = scenario->worlds_list->world_head;
    while (actual != NULL) {
        for (int i = 0; i < actual->presented_agents_number; i++) {
            if (actual->presented_agents[i]->is_infected == true) {
                actual->presented_agents[i]->is_infectious = true;
                double roll = (double) rand() / RAND_MAX;
                if (actual->presented_agents[i]->is_vaccinated == true) {
                    roll = roll * scenario->vaccine_modifier;
                }
                if (roll < scenario->lethality) {
                    actual->presented_agents[i]->is_death = 1;
                    if (scenario->verbose == true) {
                        printf("Agent %d has died at %s.\n", actual->presented_agents[i]->id, actual->name);
                    }
                } else {
                    roll = (double) rand() / RAND_MAX;
                    if (roll > scenario->duration) {
                        if (scenario->verbose == true) {
                            printf("Agent %d has recovered at %s.\n", actual->presented_agents[i]->id, actual->name);
                        }
                        actual->presented_agents[i]->is_infected = false;
                        actual->presented_agents[i]->is_infectious = false;
                    }
                }
            }
        }
        actual = actual->next;
    }
}

void spread_of_disease(simulation *scenario)
{
    location *actual = scenario->worlds_list->world_head;
    while (actual != NULL) {
        for (int i = 0; i < actual->presented_agents_number; i++) {
            if (actual->presented_agents[i]->is_infectious == true && actual->presented_agents[i]->is_death == false) {
                for (int j = 0; j < actual->presented_agents_number; j++) {
                    if (i != j && actual->presented_agents[j]->is_infected == false && actual->presented_agents[j]->is_death == false) {
                        double roll = (double) rand() / RAND_MAX;
                        double immunity = actual->presented_agents[j]->immunity;
                        if (actual->presented_agents[j]->is_vaccinated == true) {
                            immunity = immunity * scenario->vaccine_modifier;
                        }
                        if (actual->exposure * roll * scenario->infectivity > immunity) {
                            if (scenario->verbose == true) {
                                printf("Agent %d has infected agent %d at %s.\n", actual->presented_agents[i]->id, actual->presented_agents[j]->id, actual->name);
                            }
                            scenario->number_of_infections += 1;
                            actual->presented_agents[j]->is_infected = true;
                            actual->infected_there = actual->infected_there + 1;
                        }
                    }
                }
            }
        }
        actual = actual->next;
    }
}

void remove_agents_from_locations(simulation *scenario)
{
    location *actual = scenario->worlds_list->world_head;
    while (actual != NULL) {
        if (actual->presented_agents_number != 0) {
            free(actual->presented_agents);
            actual->presented_agents = NULL;
            actual->presented_agents_number = 0;
        }
        actual = actual->next;
    }
}

int load_simulation(simulation *scenario, FILE *agents, FILE *worlds)
{
    if (load_agents(scenario, agents) != 0) {
        free(scenario);
        return 1;
    }
    if (load_world(scenario, worlds) != 0) {
        free_agents(scenario->agents_list);
        free(scenario);
        return 1;
    }
    if (agents_to_list(scenario) != 0) {
        free_agents(scenario->agents_list);
        free_world(scenario->worlds_list);
        free(scenario);
        return 1;
    }
    if (locations_to_list(scenario) != 0) {
        free_agents(scenario->agents_list);
        free_world(scenario->worlds_list);
        free(scenario->agents);
        free(scenario);
        return 1;
    }
    if (move_agents_on_location(scenario) != 0) {
        free_simulalion(scenario);
        return 1;
    }
    scenario->number_of_infections = 0;
    return 0;
}

bool is_someone_infected(simulation *scenario)
{
    agent *actual = scenario->agents_list->agent_head;
    while (actual != NULL) {
        if (actual->is_infected == true && actual->is_death == false) {
            return true;
        }
        actual = actual->next;
    }
    return false;
}

bool is_someone_alive(simulation *scenario)
{
    agent *actual = scenario->agents_list->agent_head;
    while (actual != NULL) {
        if (actual->is_death == false) {
            return true;
        }
        actual = actual->next;
    }
    return false;
}

location *most_infections_locations(simulation *scenario)
{
    location *most_infectious_location = scenario->worlds_list->world_head;
    if (scenario->worlds_list->world_head == NULL) {
        return NULL;
    }
    int most_infectious = most_infectious_location->infected_there;
    location *actual = scenario->worlds_list->world_head->next;
    while (actual != NULL) {
        if (most_infectious == actual->infected_there) {
            most_infectious_location = NULL;
        }
        if (most_infectious < actual->infected_there) {
            most_infectious_location = actual;
            most_infectious = actual->infected_there;
        }
        actual = actual->next;
    }
    return most_infectious_location;
}

int get_survivors(simulation *scenario)
{
    agent *actual = scenario->agents_list->agent_head;
    int survivors = 0;
    while (actual != NULL) {
        if (actual->is_death == false) {
            survivors += 1;
        }
        actual = actual->next;
    }
    return survivors;
}

void print_statistics(simulation *scenario)
{
    printf("Statistics:\n");
    location *most_infected_location = most_infections_locations(scenario);
    printf("\tTotal infections: %d\n", scenario->number_of_infections);
    int survivors = get_survivors(scenario);
    printf("\tTotal deaths: %d\n", scenario->agents_list->number_of_agents - survivors);
    printf("\tNumber of survivors: %d\n", survivors);
    printf("Most infectious location:\n");
    if (most_infected_location == NULL) {
        printf("\tMultiple\n");
    } else {
        printf("\t- %s: %d infections\n", most_infected_location->name, most_infected_location->infected_there);
    }
}

int run_simulation(simulation *scenario)
{
    printf("Random seed: %d\n", scenario->seed);
    char *result = "Step limit expired.";
    for (int i = 0; i < scenario->max_steps; i++) {
        if (scenario->verbose == true) {
            printf("\n*** STEP %d ***\n", i + 1);
        }

        remove_agents_from_locations(scenario);

        if (move_agents_on_location(scenario) != 0) {
            free_simulalion(scenario);
            return 1;
        }
        progress_of_disease(scenario);
        spread_of_disease(scenario);
        if (i + 1 == scenario->max_steps) {
            break;
        }

        if (is_someone_alive(scenario) == false) {
            scenario->max_steps = i + 1;
            result = "Population is extinct";
            break;
        }
        if (is_someone_infected(scenario) == false) {
            result = "Virus is extinct.";
            scenario->max_steps = i + 1;
            break;
        }
    }
    printf("%s\n", result);
    print_statistics(scenario);
    printf("Simulation terminated after %d steps.\n", scenario->max_steps);
    free_simulalion(scenario);
    return 0;
}
