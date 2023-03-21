//
// Created by Petr Valik on 20.06.2022.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utilities.h"
#include "simulation.h"

void free_world(world_list *list)
{
    location *next;
    location *actual = list->world_head;
    while (actual != NULL) {
        next = actual->next;
        free(actual->name);
        if (actual->presented_agents_number != 0) {
            free(actual->presented_agents);
        }
        free(actual);
        actual = next;
    }
    free(list);
}

int add_to_world(world_list *list, location *new_location)
{
    location *actual = list->world_head;
    if (actual == NULL) {
        list->world_head = new_location;
        new_location->prev = NULL;
        list->world_tail = new_location;
        new_location->next = NULL;
    } else {
        while (actual != NULL) {
            if (actual->id == new_location->id) {
                fprintf(stderr, "%s\n", "duplicate agent id");
                return 1;
            }
            if (actual->id > new_location->id) {
                if (actual == list->world_head) {
                    list->world_head = new_location;
                    new_location->prev = NULL;
                } else {
                    actual->prev->next = new_location;
                    new_location->prev = actual->prev;
                }
                actual->prev = new_location;
                new_location->next = actual;
                return 0;
            }
            actual = actual->next;
        }
        list->world_tail->next = new_location;
        new_location->prev = list->world_tail;
        list->world_tail = new_location;
    }
    return 0;
}

int parse_location_id(location *new_location, char *id)
{
    if (id == NULL) {
        fprintf(stderr, "%s\n", "wrong csv agent arguments");
        return 1;
    }
    int id_int;
    if ((id_int = load_int(id)) < 0) {
        return 1;
    }
    new_location->id = id_int;
    return 0;
}

int parse_location_name(location *new_location, char *name)
{
    if (name == NULL) {
        fprintf(stderr, "%s\n", "wrong csv agent arguments");
        return 1;
    }
    if (strcmp("", name) == 0) {
        fprintf(stderr, "%s\n", "empty name");
        return 1;
    }
    char *copy_name = malloc(sizeof(char) * (strlen(name) + 1));
    if (copy_name == NULL) {
        fprintf(stderr, "%s\n", "wrong csv agent arguments");
        return 1;
    }
    strcpy(copy_name, name);
    new_location->name = copy_name;
    return 0;
}

int parse_exposure(location *new_location, char *exposure)
{
    if (exposure == NULL) {
        fprintf(stderr, "%s\n", "wrong csv agent arguments");
        return 1;
    }
    if (exposure[strlen(exposure) - 1] == '\n') {
        exposure[strlen(exposure) - 1] = '\0';
    }

    double exposure_float;
    if ((exposure_float = load_double(exposure)) < 0) {
        return 1;
    }
    new_location->exposure = exposure_float;
    return 0;
}

int location_parse(location *new_location, FILE *world_file)
{
    size_t length = 64;
    char *info = malloc(length + 1);
    if (info == NULL) {
        fprintf(stderr, "%s\n", "cannot allocate memory");
        return 1;
    }
    ssize_t result = getline(&info, &length, world_file);
    if (result == -1) {
        free(info);
        return 2;
    }

    // parse id done
    char *id = strtok(info, ";");
    if (parse_location_id(new_location, id) != 0) {
        free(info);
        return 1;
    }

    // parse name done
    char *name = strtok(NULL, ";");
    if (parse_location_name(new_location, name) != 0) {
        free(info);
        return 1;
    }

    // parse exposure done
    char *exposure = strtok(NULL, ";");
    if (parse_exposure(new_location, exposure) != 0) {
        free(info);
        free(new_location->name);
        return 1;
    }
    char *next = strtok(NULL, ";");
    if (next != NULL) {
        free(info);
        free(new_location->name);
        return 1;
    }
    free(info);
    return 0;
}

void null_location(location *new_location)
{
    new_location->presented_agents = NULL;
    new_location->prev = NULL;
    new_location->next = NULL;
    new_location->presented_agents_number = 0;
    new_location->infected_there = 0;
}

int load_world(simulation *scenario, FILE *world_file)
{
    int return_value = 0;
    world_list *list = malloc(sizeof(world_list));
    if (list == NULL) {
        fprintf(stderr, "%s\n", "cannot allocate memory");
        return 1;
    }
    list->world_head = NULL;
    list->world_tail = NULL;
    list->number_of_worlds = 0;
    location *new_location;
    while (return_value == 0) {
        new_location = malloc(sizeof(location));
        if (new_location == NULL) {
            free_world(list);
            return 1;
        }
        null_location(new_location);

        return_value = location_parse(new_location, world_file);
        if (return_value == 1) {
            free(new_location);
            free_world(list);
            return 1;
        } else if (return_value == 0) {
            if (add_to_world(list, new_location) != 0) {
                free(new_location->name);
                free(new_location);
                free_world(list);
                return 1;
            }
            list->number_of_worlds += 1;
        } else {
            free(new_location);
        }
    }
    scenario->worlds_list = list;
    if (scenario->worlds_list->number_of_worlds == 0) {
        free(list);
        return 1;
    }

    return 0;
}
