#ifndef PROBLEM1_H
#define PROBLEM1_H

#include "structs.h"

// Variables globals per a mètriques (definides a problem1.c)
extern long long p1_nodes_explored;

void solve_problem1_greedy(DishList *list);
// enable_pruning: true per activar la poda (optimitzat), false per força bruta pura
void solve_problem1_backtracking(DishList *list, bool enable_pruning);

#endif
