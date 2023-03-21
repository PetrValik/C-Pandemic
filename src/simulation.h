//
// Created by Petr Valik on 20.06.2022.
//

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct agent
{
    int id;
    int number_of_locations;
    int curent_location;
    int *locations;
    bool is_vaccinated;
    double immunity;
    bool is_infected;
    bool is_infectious;
    bool is_death;
    struct agent *prev;
    struct agent *next;
} agent;

typedef struct agent_list
{
    agent *agent_head;
    agent *agent_tail;
    int number_of_agents;
} agent_list;

typedef struct location
{
    int id;
    char *name;
    double exposure;
    int infected_there;
    agent **presented_agents;
    int presented_agents_number;
    struct location *prev;
    struct location *next;
} location;

typedef struct world_list
{
    location *world_head;
    location *world_tail;
    int number_of_worlds;
} world_list;

typedef struct simulation
{
    bool verbose;
    double lethality;
    double infectivity;
    double duration;
    double vaccine_modifier;
    int max_steps;
    unsigned int seed;
    int number_of_infections;
    agent **agents;
    location **worlds;
    world_list *worlds_list;
    agent_list *agents_list;
} simulation;

int load_simulation(simulation *scenario, FILE *agents, FILE *worlds);

int run_simulation(simulation *scenario);
