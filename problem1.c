#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "problem1.h"

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
    
    // Punters als plats per ordenar-los sense moure les estructures reals
    Dish *primers[100], *segons[100], *postres[100];
    int n_prim = 0, n_seg = 0, n_pos = 0;
    bool used[1000] = {false}; // Assumint que l'id màxim < 1000

    for (int i = 0; i < list->count; i++) {
        if (list->dishes[i].type == PRIMER) primers[n_prim++] = &list->dishes[i];
        else if (list->dishes[i].type == SEGON) segons[n_seg++] = &list->dishes[i];
        else if (list->dishes[i].type == POSTRE) postres[n_pos++] = &list->dishes[i];
    }

    qsort(primers, n_prim, sizeof(Dish*), compare_dishes_by_price);
    qsort(segons, n_seg, sizeof(Dish*), compare_dishes_by_price);
    qsort(postres, n_pos, sizeof(Dish*), compare_dishes_by_price);

    Menu menus[100];
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
}

// --- Estratègia de Backtracking ---

typedef struct {
    Menu *menus;
    int count;
    bool has_veg;
} Solution;

Solution best_sol;
bool *bt_used;
int bt_n_dishes;

void backtrack_menus(DishList *list, Solution *current) {
    // Comprovar si l'actual és millor que el millor
    if (current->count > best_sol.count && current->has_veg) {
        // Còpia profunda a best_sol
        if (best_sol.menus) free(best_sol.menus);
        best_sol.menus = malloc(current->count * sizeof(Menu));
        memcpy(best_sol.menus, current->menus, current->count * sizeof(Menu));
        best_sol.count = current->count;
        best_sol.has_veg = current->has_veg;
    }

    // Optimització: Poda
    // Si els plats restants no poden superar best_sol, retornar.
    // Comptar primers, segons i postres no utilitzats. Màxim teòric de menús addicionals = min(unused_prim, unused_seg, unused_postre)
    int unused_p = 0, unused_s = 0, unused_po = 0;
    for(int i=0; i<list->count; i++) {
        if(!bt_used[list->dishes[i].id]) {
            if(list->dishes[i].type == PRIMER) unused_p++;
            else if(list->dishes[i].type == SEGON) unused_s++;
            else if(list->dishes[i].type == POSTRE) unused_po++;
        }
    }
    int max_possible = current->count + (unused_p < unused_s ? (unused_p < unused_po ? unused_p : unused_po) : (unused_s < unused_po ? unused_s : unused_po));
    
    if (max_possible <= best_sol.count) return;

    // Intentar trobar una tupla (p, s, po)
    // Per evitar permutacions del mateix conjunt de menús, hauríem d'iterar en un ordre canònic o consumir plats.
    // No obstant això, com que només necessitem trobar *qualsevol* combinació que maximitzi K, podem iterar pels plats disponibles.
    // Per evitar O(N^3) en cada pas, podem agafar el primer Primer disponible, i després intentar casar-lo amb S i Po.
    
    int p_idx = -1;
    for(int i=0; i<list->count; i++) {
        if (list->dishes[i].type == PRIMER && !bt_used[list->dishes[i].id]) {
            p_idx = i;
            break; // Agafar el primer primer disponible per forçar l'ordre i reduir el factor de ramificació
        }
    }

    if (p_idx == -1) return; // No hi ha més primers, no es poden fer més menús

    bt_used[list->dishes[p_idx].id] = true;

    for (int s = 0; s < list->count; s++) {
        if (list->dishes[s].type == SEGON && !bt_used[list->dishes[s].id]) {
            bt_used[list->dishes[s].id] = true;
            
            for (int po = 0; po < list->count; po++) {
                if (list->dishes[po].type == POSTRE && !bt_used[list->dishes[po].id]) {
                     if (is_valid_menu(&list->dishes[p_idx], &list->dishes[s], &list->dishes[po])) {
                        bool is_veg = list->dishes[p_idx].is_vegetarian && 
                                      list->dishes[s].is_vegetarian && 
                                      list->dishes[po].is_vegetarian;

                        // Afegir a la solució actual
                        bt_used[list->dishes[po].id] = true;
                        
                        // Reassignar la matriu de menús actual (enfocament simple)
                        current->menus = realloc(current->menus, (current->count + 1) * sizeof(Menu));
                        current->menus[current->count].primer_idx = list->dishes[p_idx].id;
                        current->menus[current->count].segon_idx = list->dishes[s].id;
                        current->menus[current->count].postre_idx = list->dishes[po].id;
                        current->menus[current->count].total_price = list->dishes[p_idx].price + list->dishes[s].price + list->dishes[po].price;
                        current->menus[current->count].avg_popularity = (list->dishes[p_idx].popularity + list->dishes[s].popularity + list->dishes[po].popularity) / 3.0;
                        current->menus[current->count].is_vegetarian = is_veg;
                        
                        bool old_has_veg = current->has_veg;
                        if (is_veg) current->has_veg = true;
                        current->count++;

                        backtrack_menus(list, current);

                        // Backtrack (tornar enrere)
                        current->count--;
                        current->has_veg = old_has_veg; // Només funciona si assumim que un camí cap avall el va establir.
                        // De fet, has_veg és una propietat del CONJUNT. Si eliminem un menú vegetarià, podríem perdre la propietat.
                        // Hem de tornar a comprovar has_veg o comptar quants menús vegetarians tenim.
                        // Comptem els menús vegetarians en lloc de bool.
                        bt_used[list->dishes[po].id] = false;
                     }
                }
            }
            bt_used[list->dishes[s].id] = false;
        }
    }
    
    // Considerar també l'opció on saltem aquest primer?
    // Si saltem aquest primer, mai no el podrem utilitzar (ja que agafem el primer disponible).
    // De fet, si no aconseguim casar aquest primer amb CAP segon/postre, efectivament el saltem.
    // Però i si casar-lo impedeix una millor solució global?
    // Com que hem agafat el *primer* disponible, si no l'utilitzem ara, diem "aquest primer no forma part de cap menú en el conjunt òptim".
    // Així que hauríem de tenir una branca on NO utilitzem p_idx.
    
    bt_used[list->dishes[p_idx].id] = false; // Desmarcar com a utilitzat per a la branca "saltar"
    // Marcar com a "saltat" per a aquest nivell de recursivitat?
    // Enfocament estàndard: Iterar pels elements, incloure o excloure.
    // Aquí construïm menús.
    
    // Recursivitat alternativa:
    // try_match(primer_idx):
    //   if invalid primer, return
    //   branch 1: Try to match primer_idx with all valid pairs of (S, Po). For each match, recurse.
    //   branch 2: Don't use primer_idx. Recurse to try_match(primer_idx + 1).
    
}

// Millor estructura de Backtracking
void backtrack_recursive(DishList *list, int p_idx, int veg_count, int menu_count, Menu *current_menus) {
    
    // Cas base o comprovació de Poda
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


void solve_problem1_backtracking(DishList *list) {
    printf("\n--- Problem 1: Backtracking Strategy ---\n");
    
    best_sol.menus = NULL;
    best_sol.count = 0; // Corregit: Inicialitzar a 0
    best_sol.has_veg = false;

    bt_used = (bool*)calloc(1000, sizeof(bool)); // Ajustar la mida segons sigui necessari o utilitzar l'ID màxim
    
    Menu *current_menus = malloc(list->count * sizeof(Menu));
    
    backtrack_recursive(list, 0, 0, 0, current_menus);
    
    printf("Best Solution Found (Menus: %d):\n", best_sol.count);
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
