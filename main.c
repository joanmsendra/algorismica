#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "file_io.h"
#include "problem1.h"
#include "problem2.h"

int main() {
    DishList list = load_dishes("plats_m.txt");
    if (list.count == -1) {
        return 1;
    }

    printf("Loaded %d dishes.\n", list.count);

    int choice;
    do {
        printf("\n=== MENJAR AL CAMPUS - OPTIMITZACIO ===\n");
        printf("1. Menu Composition (Greedy)\n");
        printf("2. Menu Composition (Backtracking)\n");
        printf("3. Equitable Distribution (Greedy)\n");
        printf("4. Equitable Distribution (Branch & Bound)\n");
        printf("5. Run All\n");
        printf("0. Exit\n");
        printf("Select an option: ");
        if (scanf("%d", &choice) != 1) break;

        clock_t start, end;
        double cpu_time_used;

        if (choice == 1 || choice == 5) {
            start = clock();
            solve_problem1_greedy(&list);
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("Time taken: %f seconds\n", cpu_time_used);
        }
        if (choice == 2 || choice == 5) {
            start = clock();
            solve_problem1_backtracking(&list);
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("Time taken: %f seconds\n", cpu_time_used);
        }
        if (choice == 3 || choice == 5) {
            start = clock();
            solve_problem2_greedy(&list);
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("Time taken: %f seconds\n", cpu_time_used);
        }
        if (choice == 4 || choice == 5) {
            start = clock();
            solve_problem2_branch_and_bound(&list);
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("Time taken: %f seconds\n", cpu_time_used);
        }

    } while (choice != 0);

    free_dish_list(&list);
    return 0;
}
