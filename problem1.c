#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "problem1.h"

// Variable global per a mètriques
long long p1_nodes_explored = 0;
bool p1_enable_pruning = true;

// --- Funcions Auxiliars ---

void print_menu(const Menu *m, const DishList *list, int menu_num) {
    printf("Menu %d:\n", menu_num);
    printf("  Primer: %s (%.2f EUR, Pop: %d)\n", list->dishes[m->primer_idx].name, list->dishes[m->primer_idx].price, list->dishes[m->primer_idx].popularity);
    printf("  Segon:  %s (%.2f EUR, Pop: %d)\n", list->dishes[m->segon_idx].name, list->dishes[m->segon_idx].price, list->dishes[m->segon_idx].popularity);
    printf("  Postre: %s (%.2f EUR, Pop: %d)\n", list->dishes[m->postre_idx].name, list->dishes[m->postre_idx].price, list->dishes[m->postre_idx].popularity);
    printf("  Total: %.2f EUR, Avg Pop: %.2f, Veg: %s\n", m->total_price, m->avg_popularity, m->is_vegetarian ? "Yes" : "No");
}

bool is_valid_menu(const Dish *p1, const Dish *p2, const Dish *p3) {
    float price = p1->price + p2->price + p3->price;
    float avg_pop = (p1->popularity + p2->popularity + p3->popularity) / 3.0;
    return (price <= 12.0 && avg_pop >= 4.0);
}

// --- Estratègia Voraç (Greedy) ---
// Estratègia: Ordenar els plats per preu. Intentar construir primer el menú vegetarià requerit (el menú vegetarià vàlid més barat).
// Després construir tants altres menús com sigui possible amb els plats més barats restants.

int compare_dishes_by_price(const void *a, const void *b) {
    Dish *d1 = *(Dish **)a;
    Dish *d2 = *(Dish **)b;
    if (d1->price < d2->price) return -1;
    if (d1->price > d2->price) return 1;
    return 0;
}

void solve_problem1_greedy(DishList *list) {
    printf("\n--- Problem 1: Greedy Strategy ---\n");
    
    // Assignació dinàmica per suportar qualsevol mida de dataset (evita Crash amb L/XL/XXL)
    Dish **primers = malloc(list->count * sizeof(Dish*));
    Dish **segons = malloc(list->count * sizeof(Dish*));
    Dish **postres = malloc(list->count * sizeof(Dish*));
    bool *used = calloc(list->count, sizeof(bool)); // Utilitza calloc per inicialitzar a false
    Menu *menus = malloc(list->count * sizeof(Menu)); // Mida màxima teòrica

    if (!primers || !segons || !postres || !used || !menus) {
        fprintf(stderr, "Error: No hi ha prou memòria per executar Greedy.\n");
        if(primers) free(primers);
        if(segons) free(segons);
        if(postres) free(postres);
        if(used) free(used);
        if(menus) free(menus);
        return;
    }

    int n_prim = 0, n_seg = 0, n_pos = 0;
    
    for (int i = 0; i < list->count; i++) {
        if (list->dishes[i].type == PRIMER) primers[n_prim++] = &list->dishes[i];
        else if (list->dishes[i].type == SEGON) segons[n_seg++] = &list->dishes[i];
        else if (list->dishes[i].type == POSTRE) postres[n_pos++] = &list->dishes[i];
    }

    qsort(primers, n_prim, sizeof(Dish*), compare_dishes_by_price);
    qsort(segons, n_seg, sizeof(Dish*), compare_dishes_by_price);
    qsort(postres, n_pos, sizeof(Dish*), compare_dishes_by_price);

    int menu_count = 0;
    bool veg_menu_found = false;

    // 1. Intentar trobar primer un menú vegetarià
    for (int i = 0; i < n_prim; i++) {
        if (!primers[i]->is_vegetarian) continue;
        for (int j = 0; j < n_seg; j++) {
            if (!segons[j]->is_vegetarian) continue;
            for (int k = 0; k < n_pos; k++) {
                if (!postres[k]->is_vegetarian) continue;

                if (is_valid_menu(primers[i], segons[j], postres[k])) {
                    menus[menu_count].primer_idx = primers[i]->id;
                    menus[menu_count].segon_idx = segons[j]->id;
                    menus[menu_count].postre_idx = postres[k]->id;
                    menus[menu_count].total_price = primers[i]->price + segons[j]->price + postres[k]->price;
                    menus[menu_count].avg_popularity = (primers[i]->popularity + segons[j]->popularity + postres[k]->popularity) / 3.0;
                    menus[menu_count].is_vegetarian = true;
                    
                    used[primers[i]->id] = true;
                    used[segons[j]->id] = true;
                    used[postres[k]->id] = true;
                    
                    menu_count++;
                    veg_menu_found = true;
                    goto veg_found;
                }
            }
        }
    }
veg_found:

    if (!veg_menu_found) {
        printf("Could not form a valid vegetarian menu.\n");
        // Podem continuar o aturar-nos. El requisit diu "Almenys un...".
        // Si no podem, llavors 0 menús és la resposta (estrictament parlant), o simplement fem el que podem.
    }

    // 2. Omplir la resta
    for (int i = 0; i < n_prim; i++) {
        if (used[primers[i]->id]) continue;
        for (int j = 0; j < n_seg; j++) {
            if (used[segons[j]->id]) continue;
            for (int k = 0; k < n_pos; k++) {
                if (used[postres[k]->id]) continue;

                if (is_valid_menu(primers[i], segons[j], postres[k])) {
                    menus[menu_count].primer_idx = primers[i]->id;
                    menus[menu_count].segon_idx = segons[j]->id;
                    menus[menu_count].postre_idx = postres[k]->id;
                    menus[menu_count].total_price = primers[i]->price + segons[j]->price + postres[k]->price;
                    menus[menu_count].avg_popularity = (primers[i]->popularity + segons[j]->popularity + postres[k]->popularity) / 3.0;
                    menus[menu_count].is_vegetarian = primers[i]->is_vegetarian && segons[j]->is_vegetarian && postres[k]->is_vegetarian;

                    used[primers[i]->id] = true;
                    used[segons[j]->id] = true;
                    used[postres[k]->id] = true;
                    menu_count++;
                    goto next_primer; // Passar al següent primer
                }
            }
        }
        next_primer:;
    }

    printf("Total Menus Created: %d\n", menu_count);
    for (int i = 0; i < menu_count; i++) {
        print_menu(&menus[i], list, i + 1);
    }

    free(primers);
    free(segons);
    free(postres);
    free(used);
    free(menus);
}

// --- Estratègia de Backtracking ---

typedef struct {
    Menu *menus;
    int count;
    bool has_veg;
} Solution;

Solution best_sol;
bool *bt_used;

// Millor estructura de Backtracking
void backtrack_recursive(DishList *list, int p_idx, int veg_count, int menu_count, Menu *current_menus) {
    p1_nodes_explored++;

    // Cas base o comprovació de Poda
    if (p1_enable_pruning) {
        int remaining_primers = 0;
        for(int i = p_idx; i < list->count; i++) 
            if (list->dishes[i].type == PRIMER && !bt_used[list->dishes[i].id]) remaining_primers++;
        
        int unused_s = 0, unused_po = 0;
        for(int i=0; i<list->count; i++) {
            if(!bt_used[list->dishes[i].id]) {
                if(list->dishes[i].type == SEGON) unused_s++;
                else if(list->dishes[i].type == POSTRE) unused_po++;
            }
        }
        
        int potential = menu_count + (remaining_primers < unused_s ? (remaining_primers < unused_po ? remaining_primers : unused_po) : (unused_s < unused_po ? unused_s : unused_po));
        
        if (potential <= best_sol.count) return;
    }

    // Actualitzar la millor solució si és vàlida
    if (menu_count > best_sol.count && veg_count > 0) {
         if (best_sol.menus) free(best_sol.menus);
        best_sol.menus = malloc(menu_count * sizeof(Menu));
        memcpy(best_sol.menus, current_menus, menu_count * sizeof(Menu));
        best_sol.count = menu_count;
        best_sol.has_veg = true;
    }

    // Cercar el següent primer
    int next_p = -1;
    for(int i = p_idx; i < list->count; i++) {
        if (list->dishes[i].type == PRIMER && !bt_used[list->dishes[i].id]) {
            next_p = i;
            break;
        }
    }

    if (next_p == -1) return;

    // Branca 1: Intentar utilitzar aquest primer
    bt_used[list->dishes[next_p].id] = true;
    
    for (int s = 0; s < list->count; s++) {
        if (list->dishes[s].type == SEGON && !bt_used[list->dishes[s].id]) {
            bt_used[list->dishes[s].id] = true;
            for (int po = 0; po < list->count; po++) {
                if (list->dishes[po].type == POSTRE && !bt_used[list->dishes[po].id]) {
                    if (is_valid_menu(&list->dishes[next_p], &list->dishes[s], &list->dishes[po])) {
                        bt_used[list->dishes[po].id] = true;
                        
                        bool is_veg = list->dishes[next_p].is_vegetarian && list->dishes[s].is_vegetarian && list->dishes[po].is_vegetarian;
                        
                        current_menus[menu_count].primer_idx = list->dishes[next_p].id;
                        current_menus[menu_count].segon_idx = list->dishes[s].id;
                        current_menus[menu_count].postre_idx = list->dishes[po].id;
                        current_menus[menu_count].total_price = list->dishes[next_p].price + list->dishes[s].price + list->dishes[po].price;
                        current_menus[menu_count].avg_popularity = (list->dishes[next_p].popularity + list->dishes[s].popularity + list->dishes[po].popularity) / 3.0;
                        current_menus[menu_count].is_vegetarian = is_veg;

                        backtrack_recursive(list, next_p + 1, veg_count + (is_veg ? 1 : 0), menu_count + 1, current_menus);

                        bt_used[list->dishes[po].id] = false;
                    }
                }
            }
            bt_used[list->dishes[s].id] = false;
        }
    }

    // Branca 2: No utilitzar aquest primer
    // Ja l'hem marcat com a utilitzat a l'inici de la Branca 1, així que el desmarquem
    bt_used[list->dishes[next_p].id] = false;
    
    // IMPORTANT: Si saltem aquest primer, passem a cercar des de next_p + 1
    backtrack_recursive(list, next_p + 1, veg_count, menu_count, current_menus);
}


void solve_problem1_backtracking(DishList *list, bool enable_pruning) {
    p1_enable_pruning = enable_pruning;
    p1_nodes_explored = 0;
    
    printf("\n--- Problem 1: Backtracking Strategy (Pruning: %s) ---\n", enable_pruning ? "ON" : "OFF");
    
    best_sol.menus = NULL;
    best_sol.count = 0;
    best_sol.has_veg = false;

    bt_used = (bool*)calloc(list->count + 1, sizeof(bool)); // Ajustat dinàmicament segons N
    
    Menu *current_menus = malloc(list->count * sizeof(Menu));
    
    backtrack_recursive(list, 0, 0, 0, current_menus);
    
    printf("Best Solution Found (Menus: %d)\n", best_sol.count);
    printf("Nodes Explored: %I64d\n", p1_nodes_explored);

    if (best_sol.count > 0) {
        for (int i = 0; i < best_sol.count; i++) {
            print_menu(&best_sol.menus[i], list, i+1);
        }
    } else {
        printf("No valid solution found satisfying all constraints.\n");
    }

    free(bt_used);
    free(current_menus);
    if (best_sol.menus) free(best_sol.menus);
}
