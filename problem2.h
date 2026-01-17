#ifndef PROBLEM2_H
#define PROBLEM2_H

#include "structs.h"

// Variables globals per a mètriques
extern long long p2_nodes_explored;

void solve_problem2_greedy(DishList *list);
// enable_sorting: true per ordenar els plats (heurística), false per ordre original
void solve_problem2_branch_and_bound(DishList *list, bool enable_sorting);

#endif
