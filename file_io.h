#ifndef FILE_IO_H
#define FILE_IO_H

#include "structs.h"

// Carrega els plats d'un fitxer. Retorna una DishList.
// Si la càrrega falla, count serà -1.
DishList load_dishes(const char *filename);

// Allibera la memòria assignada per a la llista de plats.
void free_dish_list(DishList *list);

#endif
