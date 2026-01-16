#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "problem2.h"

#define NUM_CAFETERIAS 3

// --- Funcions Auxiliars ---

void init_cafeteria(Cafeteria *c, int max_dishes) {
    c->dish_indices = (int *)malloc(max_dishes * sizeof(int));
    c->count = 0;
    c->capacity = max_dishes;
    c->total_popularity = 0;
    c->has_primer = false;
    c->has_segon = false;
    c->has_postre = false;
    c->veg_count = 0;
}

void free_cafeteria(Cafeteria *c) {
    if (c->dish_indices) free(c->dish_indices);
}

void print_distribution(Cafeteria cafs[], DishList *list) {
    for (int i = 0; i < NUM_CAFETERIAS; i++) {
        printf("Cafeteria %d (Pop: %d, Veg: %d):\n", i + 1, cafs[i].total_popularity, cafs[i].veg_count);
        for (int j = 0; j < cafs[i].count; j++) {
            int d_idx = cafs[i].dish_indices[j];
            printf("  - %s (%s, Veg: %s, Pop: %d)\n", 
                   list->dishes[d_idx].name, 
                   list->dishes[d_idx].type == PRIMER ? "Primer" : (list->dishes[d_idx].type == SEGON ? "Segon" : "Postre"),
                   list->dishes[d_idx].is_vegetarian ? "Yes" : "No",
                   list->dishes[d_idx].popularity);
        }
    }
}

int calculate_imbalance(Cafeteria cafs[]) {
    int min_pop = INT_MAX, max_pop = INT_MIN;
    for (int i = 0; i < NUM_CAFETERIAS; i++) {
        if (cafs[i].total_popularity < min_pop) min_pop = cafs[i].total_popularity;
        if (cafs[i].total_popularity > max_pop) max_pop = cafs[i].total_popularity;
    }
    return max_pop - min_pop;
}

bool check_constraints(Cafeteria cafs[], int total_veg) {
    (void)total_veg; // No utilitzat
    // "Equitablement" normalment significa diferència <= 1
    // és a dir, floor(total/3) o ceil(total/3)
    
    for (int i = 0; i < NUM_CAFETERIAS; i++) {
        if (!cafs[i].has_primer || !cafs[i].has_segon || !cafs[i].has_postre) return false;
        // Comprovar l'equilibri vegetarià estrictament?
        // Permetem un equilibri fluix: desviació de la mitjana <= 1
        // De fet, el més simple és max_veg - min_veg <= 1
    }
    
    int min_v = INT_MAX, max_v = INT_MIN;
    for(int i=0; i<NUM_CAFETERIAS; i++) {
        if(cafs[i].veg_count < min_v) min_v = cafs[i].veg_count;
        if(cafs[i].veg_count > max_v) max_v = cafs[i].veg_count;
    }
    if (max_v - min_v > 1) return false;

    return true;
}

// --- Estratègia Voraç (Greedy) ---

int compare_dishes_desc_pop(const void *a, const void *b) {
    Dish *d1 = *(Dish **)a;
    Dish *d2 = *(Dish **)b;
    return d2->popularity - d1->popularity; // Descendent
}

void solve_problem2_greedy(DishList *list) {
    printf("\n--- Problem 2: Greedy Strategy ---\n");
    
    Cafeteria cafs[NUM_CAFETERIAS];
    for (int i = 0; i < NUM_CAFETERIAS; i++) init_cafeteria(&cafs[i], list->count);

    Dish *ptrs[100]; // Assumint max < 100 per a matriu estàtica, o malloc
    if (list->count > 100) { fprintf(stderr, "Increase buffer\n"); return; }

    int total_veg = 0;
    for (int i = 0; i < list->count; i++) {
        ptrs[i] = &list->dishes[i];
        if (list->dishes[i].is_vegetarian) total_veg++;
    }
    
    qsort(ptrs, list->count, sizeof(Dish*), compare_dishes_desc_pop);

    // Lògica de distribució:
    // 1. Dividir en Vegetarià i No Vegetarià
    // 2. Ordenar ambdós per Popularitat Descendent
    // 3. Distribuir plats vegetarians (Prioritzar necessitats de Tipus, després equilibri de recompte Veg, després equilibri Pop)
    // 4. Distribuir plats No vegetarians (Prioritzar necessitats de Tipus, després equilibri Pop)

    Dish *veg_dishes[100], *non_veg_dishes[100];
    int n_veg = 0, n_non = 0;

    for (int i = 0; i < list->count; i++) {
        if (ptrs[i]->is_vegetarian) veg_dishes[n_veg++] = ptrs[i];
        else non_veg_dishes[n_non++] = ptrs[i];
    }
    
    // La lògica d'ordenació ja ha ordenat els ptrs, però els hem dividit. Mantenen l'ordre relatiu si és estable,
    // o podem ordenar-los de nou. És prou simple ordenar de nou o confiar que estan ordenats si hem recorregut ptrs ordenats.
    // Sí, recórrer ptrs ordenats assegura que els sub-arrays estan ordenats.
    // Espera, he iterat list->count (0..N) no ptrs. Culpa meva.
    // Necessito iterar ptrs per mantenir l'ordre ordenat.
    
    n_veg = 0; n_non = 0;
    for (int i = 0; i < list->count; i++) {
        if (ptrs[i]->is_vegetarian) veg_dishes[n_veg++] = ptrs[i];
        else non_veg_dishes[n_non++] = ptrs[i];
    }

    // Distribuir Vegetarià
    for (int i = 0; i < n_veg; i++) {
        int best_caf = -1;
        long long best_score = LLONG_MAX;

        for (int c = 0; c < NUM_CAFETERIAS; c++) {
            bool needs = false;
            if (veg_dishes[i]->type == PRIMER && !cafs[c].has_primer) needs = true;
            if (veg_dishes[i]->type == SEGON && !cafs[c].has_segon) needs = true;
            if (veg_dishes[i]->type == POSTRE && !cafs[c].has_postre) needs = true;

            // Puntuació: Minimitzar recompte Veg (primari), Maximitzar necessitats (secundari), Minimitzar Pop (terciari)
            // Puntuació = VegCount * 100000 - (needs ? 10000 : 0) + TotalPop
            long long score = (long long)cafs[c].veg_count * 100000 
                            - (needs ? 10000 : 0) 
                            + cafs[c].total_popularity;
            
            if (score < best_score) {
                best_score = score;
                best_caf = c;
            }
        }
        
        cafs[best_caf].dish_indices[cafs[best_caf].count++] = veg_dishes[i]->id;
        cafs[best_caf].total_popularity += veg_dishes[i]->popularity;
        cafs[best_caf].veg_count++;
        if (veg_dishes[i]->type == PRIMER) cafs[best_caf].has_primer = true;
        if (veg_dishes[i]->type == SEGON) cafs[best_caf].has_segon = true;
        if (veg_dishes[i]->type == POSTRE) cafs[best_caf].has_postre = true;
    }

    // Distribuir No Vegetarià
    for (int i = 0; i < n_non; i++) {
        int best_caf = -1;
        long long best_score = LLONG_MAX;

        for (int c = 0; c < NUM_CAFETERIAS; c++) {
            bool needs = false;
            if (non_veg_dishes[i]->type == PRIMER && !cafs[c].has_primer) needs = true;
            if (non_veg_dishes[i]->type == SEGON && !cafs[c].has_segon) needs = true;
            if (non_veg_dishes[i]->type == POSTRE && !cafs[c].has_postre) needs = true;

            // Puntuació: Maximitzar necessitats (primari), Minimitzar Pop (secundari)
            // El recompte vegetarià no importa per als plats no vegetarians
            long long score = -(needs ? 100000 : 0) + cafs[c].total_popularity;
            
            if (score < best_score) {
                best_score = score;
                best_caf = c;
            }
        }

        cafs[best_caf].dish_indices[cafs[best_caf].count++] = non_veg_dishes[i]->id;
        cafs[best_caf].total_popularity += non_veg_dishes[i]->popularity;
        if (non_veg_dishes[i]->type == PRIMER) cafs[best_caf].has_primer = true;
        if (non_veg_dishes[i]->type == SEGON) cafs[best_caf].has_segon = true;
        if (non_veg_dishes[i]->type == POSTRE) cafs[best_caf].has_postre = true;
    }

    print_distribution(cafs, list);
    printf("Imbalance (Max-Min Pop): %d\n", calculate_imbalance(cafs));
    
    // Avisar si les restriccions fallen (Greedy no ho garanteix)
    if (!check_constraints(cafs, total_veg)) {
        printf("WARNING: Greedy strategy failed to satisfy all constraints.\n");
    }

    for (int i = 0; i < NUM_CAFETERIAS; i++) free_cafeteria(&cafs[i]);
}

// --- Estratègia de Ramificació i Poda (Branch & Bound) ---

int best_imbalance = INT_MAX;
int *best_assignment = NULL; // Mida de la matriu list->count, valor 0,1,2

// Auxiliar per comprovar la viabilitat parcial
bool is_promising(DishList *list, int current_idx, Cafeteria cafs[], int total_veg) {
    (void)total_veg;
    // Comprovar l'equilibri vegetarià fins ara?
    // Difícil de comprovar estrictament durant el parcial.
    // Comprovar si és IMPOSSIBLE satisfer els tipus?
    // p. ex., si la Caf 0 necessita Primer, i no queden Primers a la llista[current_idx...end], llavors podar.
    
    // Comptar tipus restants
    int rem_p = 0, rem_s = 0, rem_po = 0, rem_veg = 0;
    for (int i = current_idx; i < list->count; i++) {
        if (list->dishes[i].type == PRIMER) rem_p++;
        if (list->dishes[i].type == SEGON) rem_s++;
        if (list->dishes[i].type == POSTRE) rem_po++;
        if (list->dishes[i].is_vegetarian) rem_veg++;
    }

    for (int c = 0; c < NUM_CAFETERIAS; c++) {
        if (!cafs[c].has_primer && rem_p == 0) return false;
        if (!cafs[c].has_segon && rem_s == 0) return false;
        if (!cafs[c].has_postre && rem_po == 0) return false;
    }

    // Comprovar límit Veg
    int min_v = INT_MAX, max_v = INT_MIN;
    for(int c=0; c<NUM_CAFETERIAS; c++) {
        if(cafs[c].veg_count < min_v) min_v = cafs[c].veg_count;
        if(cafs[c].veg_count > max_v) max_v = cafs[c].veg_count;
    }
    // Si la diferència actual ja és > 1 i no la podem arreglar?
    // Podem arreglar-ho si afegim al mínim.
    // Veg teòric màxim per a la caf min_v = min_v + rem_veg.
    // Si min_v + rem_veg < max_v - 1 (aprox), llavors fallar.
    if (min_v + rem_veg < max_v - 1) return false;

    // Límit en el desequilibri de popularitat
    // El desequilibri actual és max_pop - min_pop.
    // Podem millorar-ho? Sí, afegint a min_pop.
    // Estimació del límit inferior:
    // Mitjana ideal = (suma(actual) + suma(restant)) / 3.
    // Si el màxim actual > Mitjana ideal + marge?
    // Límit simple: si (current_max - (current_min + sum_remaining_pop)) > best_imbalance, llavors podar?
    // Només si current_min + remaining < current_max - best_imbalance.
    
    int sum_rem = 0;
    for (int i = current_idx; i < list->count; i++) sum_rem += list->dishes[i].popularity;
    
    int curr_min = INT_MAX, curr_max = INT_MIN;
    for (int c = 0; c < NUM_CAFETERIAS; c++) {
        if (cafs[c].total_popularity < curr_min) curr_min = cafs[c].total_popularity;
        if (cafs[c].total_popularity > curr_max) curr_max = cafs[c].total_popularity;
    }

    // Optimista: aboquem tota la pop restant al cubell mínim.
    // Fins i tot llavors, si max - (min + sum_rem) > best_imbalance, podar.
    // Perquè la diferència només pot disminuir si min creix. Max no s'encongeix.
    if (curr_max - (curr_min + sum_rem) >= best_imbalance) return false; 
    // Espera, si best_imbalance és INT_MAX, això passa.

    // Si la diferència actual (sense afegir res) és > best_imbalance?
    // No, afegir coses podria augmentar min sense augmentar max.
    
    return true;
}

void bb_recursive(DishList *list, int idx, Cafeteria cafs[], int *assignment, int total_veg) {
    if (idx == list->count) {
        if (check_constraints(cafs, total_veg)) {
            int imb = calculate_imbalance(cafs);
            if (imb < best_imbalance) {
                best_imbalance = imb;
                for(int i=0; i<list->count; i++) best_assignment[i] = assignment[i];
            }
        }
        return;
    }

    // Poda
    if (!is_promising(list, idx, cafs, total_veg)) return;

    Dish *d = &list->dishes[idx];

    // Branca: Provar cada cafeteria
    for (int c = 0; c < NUM_CAFETERIAS; c++) {
        // Optimització: si totes les cafeteries són idèntiques (buides), només provar la primera per trencar la simetria
        // (Només rellevant al principi, o si n'hi ha diverses buides)
        if (c > 0 && cafs[c].count == 0 && cafs[c-1].count == 0) continue; 

        cafs[c].dish_indices[cafs[c].count++] = d->id;
        cafs[c].total_popularity += d->popularity;
        bool old_p = cafs[c].has_primer;
        bool old_s = cafs[c].has_segon;
        bool old_po = cafs[c].has_postre;
        if (d->type == PRIMER) cafs[c].has_primer = true;
        if (d->type == SEGON) cafs[c].has_segon = true;
        if (d->type == POSTRE) cafs[c].has_postre = true;
        if (d->is_vegetarian) cafs[c].veg_count++;
        
        assignment[idx] = c;

        bb_recursive(list, idx + 1, cafs, assignment, total_veg);

        // Backtrack (tornar enrere)
        if (d->is_vegetarian) cafs[c].veg_count--;
        cafs[c].has_postre = old_po;
        cafs[c].has_segon = old_s;
        cafs[c].has_primer = old_p;
        cafs[c].total_popularity -= d->popularity;
        cafs[c].count--;
    }
}

void solve_problem2_branch_and_bound(DishList *list) {
    printf("\n--- Problem 2: Branch & Bound Strategy ---\n");
    
    best_imbalance = INT_MAX;
    best_assignment = (int *)malloc(list->count * sizeof(int));
    int *current_assignment = (int *)malloc(list->count * sizeof(int));

    Cafeteria cafs[NUM_CAFETERIAS];
    for (int i = 0; i < NUM_CAFETERIAS; i++) init_cafeteria(&cafs[i], list->count);

    int total_veg = 0;
    for(int i=0; i<list->count; i++) if(list->dishes[i].is_vegetarian) total_veg++;

    // Ordenar la llista per popularitat descendent per ajudar B&B a trobar bones solucions aviat?
    // Nota: ordenar la llista canvia els índexs. Cal anar amb compte si confiem en els IDs.
    // L'estructura té 'id', així que estem segurs si utilitzem això.
    // De fet, ordenar ajuda al Greedy, i ajuda a la poda de B&B (assignant elements grans primer).
    // Reutilitzaré la lògica ordenada o simplement ordenaré la llista mateixa (copiar-la).
    // De moment, executem en l'ordre original o potser ordenem una còpia local de punters.
    // Per simplificar el codi B&B, utilitzem la llista tal com està. Però ordenar és una gran optimització per a B&B.
    
    // Lògica d'ordenació (copiar llista a llista temporal ordenada)
    DishList sorted_list;
    sorted_list.count = list->count;
    sorted_list.dishes = malloc(list->count * sizeof(Dish));
    memcpy(sorted_list.dishes, list->dishes, list->count * sizeof(Dish));
    
    // Ordenació simple bombolla o qsort
    for(int i=0; i<sorted_list.count-1; i++) {
        for(int j=0; j<sorted_list.count-i-1; j++) {
            if(sorted_list.dishes[j].popularity < sorted_list.dishes[j+1].popularity) {
                Dish temp = sorted_list.dishes[j];
                sorted_list.dishes[j] = sorted_list.dishes[j+1];
                sorted_list.dishes[j+1] = temp;
            }
        }
    }

    bb_recursive(&sorted_list, 0, cafs, current_assignment, total_veg);

    if (best_imbalance == INT_MAX) {
        printf("No valid distribution found.\n");
    } else {
        // Reconstruir la millor solució
        for(int i=0; i<NUM_CAFETERIAS; i++) {
             // Reiniciar cafs per imprimir
             cafs[i].count = 0;
             cafs[i].total_popularity = 0;
             cafs[i].veg_count = 0;
        }

        for(int i=0; i<sorted_list.count; i++) {
            int c = best_assignment[i];
            cafs[c].dish_indices[cafs[c].count++] = sorted_list.dishes[i].id; // Utilitzar l'ID del plat ordenat
            cafs[c].total_popularity += sorted_list.dishes[i].popularity;
            if(sorted_list.dishes[i].is_vegetarian) cafs[c].veg_count++;
        }
        
        // Imprimir utilitzant la cerca de la llista original si cal, però tenim IDs.
        // La funció d'impressió utilitza la llista i cerca per ID?
        // No, la funció d'impressió utilitza list->dishes[d_idx].
        // d_idx és id.
        // La meva llista passada a solve_... té plats a índex == id (0 a N-1).
        // Sí, load_dishes estableix id = i.
        
        print_distribution(cafs, list);
        printf("Best Imbalance: %d\n", best_imbalance);
    }

    free(sorted_list.dishes);
    free(best_assignment);
    free(current_assignment);
    for (int i = 0; i < NUM_CAFETERIAS; i++) free_cafeteria(&cafs[i]);
}
