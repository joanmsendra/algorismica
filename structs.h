#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdbool.h>

#define MAX_NAME_LEN 50
#define MAX_TYPE_LEN 20

typedef enum {
    PRIMER,
    SEGON,
    POSTRE,
    UNKNOWN_TYPE
} DishType;

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    float price;
    int calories;
    int popularity;
    bool is_vegetarian;
    DishType type;
} Dish;

typedef struct {
    Dish *dishes;
    int count;
} DishList;

typedef struct {
    int primer_idx; // Index in the main DishList
    int segon_idx;
    int postre_idx;
    float total_price;
    bool is_vegetarian;
    float avg_popularity;
} Menu;

typedef struct {
    int *dish_indices; // Indices of dishes assigned to this cafeteria
    int count;
    int capacity; // For dynamic array resizing
    int total_popularity;
    bool has_primer;
    bool has_segon;
    bool has_postre;
    int veg_count;
} Cafeteria;

#endif
