//
// Created by Petr Valik on 20.06.2022.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utilities.h"

int load_int(char *arg)
{
    char *non_numerical;
    long new_value = strtol(arg, &non_numerical, 10);
    if (strcmp(non_numerical, "") != 0 || new_value < 0 || strcmp(arg, "") == 0) {
        fprintf(stderr, "%s\n", "argument is not int");
        return -1;
    }
    return new_value;
}

double load_double(char *arg)
{
    char *non_numerical;
    double new_value = strtof(arg, &non_numerical);
    if (strcmp(non_numerical, "") != 0 || new_value < 0 || strcmp(arg, "") == 0) {
        fprintf(stderr, "%s\n", "argument is not float");
        return -1.0;
    }
    return new_value;
}
