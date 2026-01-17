#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "file_io.h"
#include "problem1.h"
#include "problem2.h"

int main() {
    DishList list = load_dishes("plats_s.txt");
    if (list.count == -1) {
        return 1;
    }

    printf("Loaded %d dishes.\n", list.count);  

    int choice;
    do {
        printf("\n=== MENJAR AL CAMPUS - OPTIMITZACIO ===\n");
        printf("1. Menu Composition (Greedy)\n");
        printf("2. Menu Composition (Backtracking Optimitzat)\n");
        printf("3. Equitable Distribution (Greedy)\n");
        printf("4. Equitable Distribution (B&B Optimitzat)\n");
        printf("5. Executar Comparativa Completa (BENCHMARK)\n");
        printf("0. Exit\n");
        printf("Select an option: ");
        if (scanf("%d", &choice) != 1) break;

        clock_t start, end;
        double cpu_time_used;

        if (choice == 1) {
            start = clock();
            solve_problem1_greedy(&list);
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("Time taken: %f seconds\n", cpu_time_used);
        }
        if (choice == 2) {
            start = clock();
            solve_problem1_backtracking(&list, true); // true = amb poda
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("Time taken: %f seconds\n", cpu_time_used);
        }
        if (choice == 3) {
            start = clock();
            solve_problem2_greedy(&list);
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("Time taken: %f seconds\n", cpu_time_used);
        }
        if (choice == 4) {
            start = clock();
            solve_problem2_branch_and_bound(&list, true); // true = amb ordenació
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("Time taken: %f seconds\n", cpu_time_used);
        }
        if (choice == 5) {
            printf("\n=== BENCHMARK START ===\n");
            
            // P1: Greedy
            start = clock();
            solve_problem1_greedy(&list);
            end = clock();
            printf("P1 Greedy Time: %f s\n", ((double) (end - start)) / CLOCKS_PER_SEC);

            // P1: Backtracking (Sense Poda vs Amb Poda)
            printf("\n--- Comparativa P1 Backtracking ---\n");
            
            // Sense Poda (Compte! Pot trigar molt si N > 15)
            if (list.count <= 15) {
                start = clock();
                solve_problem1_backtracking(&list, false);
                end = clock();
                printf("P1 Backtracking (NO PODA) Time: %f s\n", ((double) (end - start)) / CLOCKS_PER_SEC);
            } else {
                printf("Skipping P1 Backtracking (NO PODA) because N > 15 (too slow)\n");
            }

            start = clock();
            solve_problem1_backtracking(&list, true);
            end = clock();
            printf("P1 Backtracking (WITH PODA) Time: %f s\n", ((double) (end - start)) / CLOCKS_PER_SEC);

            // P2: Greedy
            start = clock();
            solve_problem2_greedy(&list);
            end = clock();
            printf("P2 Greedy Time: %f s\n", ((double) (end - start)) / CLOCKS_PER_SEC);

            // P2: B&B (Sense Ordenar vs Amb Ordenar)
            printf("\n--- Comparativa P2 Branch & Bound ---\n");
            
            start = clock();
            solve_problem2_branch_and_bound(&list, false);
            end = clock();
            printf("P2 B&B (NO SORT) Time: %f s\n", ((double) (end - start)) / CLOCKS_PER_SEC);

            start = clock();
            solve_problem2_branch_and_bound(&list, true);
            end = clock();
            printf("P2 B&B (SORTED) Time: %f s\n", ((double) (end - start)) / CLOCKS_PER_SEC);
            
            printf("\n=== BENCHMARK END ===\n");
        }

    } while (choice != 0);

    free_dish_list(&list);
    return 0;
}
