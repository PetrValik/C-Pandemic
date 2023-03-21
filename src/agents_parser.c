//
// Created by Petr Valik on 20.06.2022.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utilities.h"
#include "simulation.h"

void free_agents(agent_list *list)
{
    agent *next;
    agent *actual = list->agent_head;
    while (actual != NULL) {
        next = actual->next;
        if (actual->locations != NULL) {
            free(actual->locations);
        }
        free(actual);
        actual = next;
    }
    free(list);
}

int add_to_agent_list(agent_list *list, agent *new_agent)
{
    agent *actual = list->agent_head;
    if (actual == NULL) {
        list->agent_head = new_agent;
        new_agent->prev = NULL;
        list->agent_tail = new_agent;
        new_agent->next = NULL;
    } else {
        while (actual != NULL) {
            if (actual->id == new_agent->id) {
                fprintf(stderr, "%s\n", "duplicate agent id");
                return 1;
            }
            if (actual->id > new_agent->id) {
                if (actual == list->agent_head) {
                    list->agent_head = new_agent;
                    new_agent->prev = NULL;
                } else {
                    actual->prev->next = new_agent;
                    new_agent->prev = actual->prev;
                }
                actual->prev = new_agent;
                new_agent->next = actual;
                return 0;
            }
            actual = actual->next;
        }
        list->agent_tail->next = new_agent;
        new_agent->prev = list->agent_tail;
        list->agent_tail = new_agent;
    }
    return 0;
}

int parse_agent_locations(agent *new_agent, char *locations)
{
    int list_len = 1;
    int location_int;
    int *new_agent_locations;
    int *agent_locations = malloc(sizeof(int) * list_len);
    if (agent_locations == NULL) {
        fprintf(stderr, "%s\n", "cannot allocate memory");
        return 1;
    }
    char *token;
    token = strtok(locations, "-");
    if (token == NULL) {
        fprintf(stderr, "%s\n", "empty argument");
        return 1;
    }
    if ((location_int = load_int(token)) < 0) {
        fprintf(stderr, "%s\n", "negative id");
        return 1;
    }
    agent_locations[list_len - 1] = location_int;
    while (true) {
        list_len += 1;
        token = strtok(NULL, "-");
        if (token == NULL) {
            break;
        }
        if ((location_int = load_int(token)) < 0) {
            fprintf(stderr, "%s\n", "negative id");
            return 1;
        }
        new_agent_locations = realloc(agent_locations, sizeof(int) * list_len);
        if (new_agent_locations == NULL) {
            free(agent_locations);
            fprintf(stderr, "%s\n", "cannot allocate memory");
            return 1;
        }
        agent_locations = new_agent_locations;
        agent_locations[list_len - 1] = location_int;
    }
    new_agent->locations = agent_locations;
    new_agent->number_of_locations = list_len - 1;
    new_agent->curent_location = 0;
    return 0;
}

int parse_agent_id(agent *new_agent, char *id)
{
    if (id == NULL) {
        fprintf(stderr, "%s\n", "wrong csv agent arguments");
        return 1;
    }
    int id_int;
    if ((id_int = load_int(id)) < 0) {
        return 1;
    }
    new_agent->id = id_int;
    return 0;
}

int parse_is_vaccinated(agent *new_agent, char *is_vaccinated)
{
    if (is_vaccinated == NULL) {
        fprintf(stderr, "%s\n", "wrong csv agent arguments");
        return 1;
    }
    int is_vaccinated_int = load_int(is_vaccinated);
    if (is_vaccinated_int != 0 && is_vaccinated_int != 1) {
        return 1;
    }

    if (is_vaccinated_int == 0) {
        new_agent->is_vaccinated = false;
    } else {
        new_agent->is_vaccinated = true;
    }
    return 0;
}

int parse_immunity(agent *new_agent, char *immunity)
{
    if (immunity == NULL) {
        fprintf(stderr, "%s\n", "wrong csv agent arguments");
        return 1;
    }
    double immunity_float;
    if ((immunity_float = load_double(immunity)) < 0) {
        return 1;
    }
    if (immunity_float > 1.0 || immunity_float < 0.0) {
        return 1;
    }
    new_agent->immunity = immunity_float;
    return 0;
}

int parse_is_infected(agent *new_agent, char *is_infected)
{
    if (is_infected == NULL) {
        fprintf(stderr, "%s\n", "wrong csv agent arguments");
        return 1;
    }
    if (is_infected[strlen(is_infected) - 1] == '\n') {
        is_infected[strlen(is_infected) - 1] = '\0';
    }
    int int_is_infected = load_int(is_infected);
    if (int_is_infected != 0 && int_is_infected != 1) {
        return 1;
    }
    if (int_is_infected == 1) {
        new_agent->is_infected = true;
    } else {
        new_agent->is_infected = false;
    }
    return 0;
}

int agent_parse(agent *new_agent, FILE *agent_file)
{
    size_t length = 64;
    char *info = malloc(length + 1);
    if (info == NULL) {
        fprintf(stderr, "%s\n", "cannot allocate memory");
        return 1;
    }
    ssize_t result = getline(&info, &length, agent_file);
    if (result == -1) {
        free(info);
        return 2;
    }
    char *second_part = malloc(strlen(info) + 1);
    if (second_part == NULL) {
        fprintf(stderr, "%s\n", "cannot allocate memory");
        free(info);
        return 1;
    }
    strcpy(second_part, info);
    //parse id done
    char *id = strtok(info, ";");
    if (parse_agent_id(new_agent, id) != 0) {
        free(second_part);
        free(info);
        return 1;
    }

    //parse location done
    char *locations = strtok(NULL, ";");
    if (locations == NULL) {
        fprintf(stderr, "%s\n", "wrong csv agent arguments");
        free(second_part);
        free(info);
        return 1;
    }

    if (parse_agent_locations(new_agent, locations) != 0) {
        free(second_part);
        free(info);
        return 1;
    }
    // skip to third column
    strtok(second_part, ";");
    strtok(NULL, ";");

    //parse is_vaccinated done
    char *is_vaccinated = strtok(NULL, ";");
    if (parse_is_vaccinated(new_agent, is_vaccinated) != 0) {
        free(second_part);
        free(info);
        free(new_agent->locations);
        return 1;
    }

    // parse immunity done
    char *immunity = strtok(NULL, ";");
    if (parse_immunity(new_agent, immunity) != 0) {
        free(second_part);
        free(info);
        free(new_agent->locations);
        return 1;
    }
    // parse is infected done
    char *is_infected = strtok(NULL, ";");
    if (parse_is_infected(new_agent, is_infected) != 0) {
        free(second_part);
        free(info);
        free(new_agent->locations);
        return 1;
    }

    char *next = strtok(NULL, ";");
    if (next != NULL) {
        free(second_part);
        free(info);
        free(new_agent->locations);
        return 1;
    }

    free(second_part);
    free(info);
    return 0;
}

int load_agents(simulation *scenario, FILE *agent_file)
{
    int return_value = 0;
    agent_list *list = malloc(sizeof(agent_list));
    if (list == NULL) {
        fprintf(stderr, "%s\n", "cannot allocate memory");
        return 1;
    }
    list->agent_head = NULL;
    list->agent_tail = NULL;
    list->number_of_agents = 0;
    agent *new_agent;
    while (return_value == 0) {
        new_agent = malloc(sizeof(agent));
        if (new_agent == NULL) {
            free_agents(list);
            return 1;
        }
        new_agent->is_death = false;
        new_agent->is_infectious = false;
        new_agent->prev = NULL;
        new_agent->next = NULL;
        return_value = agent_parse(new_agent, agent_file);
        if (return_value == 1) {
            free(new_agent);
            free_agents(list);
            return 1;
        } else if (return_value == 0) {
            if (add_to_agent_list(list, new_agent) != 0) {
                free(new_agent->locations);
                free(new_agent);
                free_agents(list);
                return 1;
            }
            list->number_of_agents += 1;
        } else {
            free(new_agent);
        }
    }
    scenario->agents_list = list;
    if (scenario->agents_list->number_of_agents == 0) {
        free(list);
        return 1;
    }

    return 0;
}
