#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_io.h"

DishType parse_type(const char *type_str) {
    if (strcmp(type_str, "primer") == 0) return PRIMER;
    if (strcmp(type_str, "segon") == 0) return SEGON;
    if (strcmp(type_str, "postre") == 0) return POSTRE;
    return UNKNOWN_TYPE;
}

DishList load_dishes(const char *filename) {
    DishList list;
    list.dishes = NULL;
    list.count = 0;

    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Error opening file");
        list.count = -1;
        return list;
    }

    int n_plats;
    if (fscanf(f, "%d\n", &n_plats) != 1) {
        fprintf(stderr, "Error reading number of dishes\n");
        fclose(f);
        list.count = -1;
        return list;
    }

    list.dishes = (Dish *)malloc(n_plats * sizeof(Dish));
    if (!list.dishes) {
        fprintf(stderr, "Memory allocation error\n");
        fclose(f);
        list.count = -1;
        return list;
    }

    list.count = n_plats;
    char line[256];
    int i = 0;

    while (i < n_plats && fgets(line, sizeof(line), f)) {
        // Eliminar el salt de línia
        line[strcspn(line, "\n")] = 0;

        char *token = strtok(line, ";");
        if (!token) continue;
        strncpy(list.dishes[i].name, token, MAX_NAME_LEN - 1);
        list.dishes[i].name[MAX_NAME_LEN - 1] = '\0';
        list.dishes[i].id = i;

        token = strtok(NULL, ";");
        list.dishes[i].price = token ? atof(token) : 0.0;

        token = strtok(NULL, ";");
        list.dishes[i].calories = token ? atoi(token) : 0;

        token = strtok(NULL, ";");
        list.dishes[i].popularity = token ? atoi(token) : 0;

        token = strtok(NULL, ";");
        list.dishes[i].is_vegetarian = (token && strcmp(token, "true") == 0);

        token = strtok(NULL, ";");
        list.dishes[i].type = token ? parse_type(token) : UNKNOWN_TYPE;

        i++;
    }

    fclose(f);
    return list;
}

void free_dish_list(DishList *list) {
    if (list->dishes) {
        free(list->dishes);
        list->dishes = NULL;
    }
    list->count = 0;
}
