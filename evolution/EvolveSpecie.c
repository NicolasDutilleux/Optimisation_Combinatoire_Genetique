// evolution/EvolveSpecie.c
// =============================================================================
// GENETIC EVOLUTION FOR A SINGLE SPECIES
// =============================================================================

#include "EvolveSpecie.h"
#include "cost\Cost.h"
#include "genetic\Mutation.h"
#include "genetic\Crossover.h"
#include "genetic\Selection.h"
#include "local_search\TwoOpt.h"
#include "utils\Random.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

// =============================================================================
// SORTING HELPER
// =============================================================================
typedef struct {
    double cost;
    int index;
} SortEntry;

static int compare_by_cost(const void* a, const void* b) {
    double diff = ((SortEntry*)a)->cost - ((SortEntry*)b)->cost;
    if (diff < 0) return -1;
    if (diff > 0) return 1;
    return 0;
}

// =============================================================================
// MAIN EVOLUTION FUNCTION
// =============================================================================
void EvolveSpecie(
    Individual* specie,
    int specie_size,
    const double** dist,
    const int** ranking,
    const Node* nodes,
    int num_nodes,
    int alpha,
    int total_stations,
    double mutation_rate,
    int elitism_count,
    int add_pct,
    int remove_pct,
    int swap_pct,
    int inv_pct,
    int scr_pct,
    double mating_pool_fraction,
    int enable_logs,
    int enable_timers)
{
    if (!specie || specie_size <= 0) return;

    // =========================================================================
    // STEP 1: EVALUATE ALL INDIVIDUALS
    // =========================================================================
    double* costs = (double*)malloc(specie_size * sizeof(double));
    if (!costs) return;

    for (int i = 0; i < specie_size; i++) {
        if (!specie[i].active_ring) {
            Individual_Init(&specie[i], total_stations);
        }
        
        if (specie[i].cached_cost < 1e17) {
            costs[i] = specie[i].cached_cost;
        } else {
            costs[i] = Total_Cost_Individual(alpha, &specie[i], total_stations, dist, ranking);
            specie[i].cached_cost = costs[i];
        }
    }

    // =========================================================================
    // STEP 2: SORT BY FITNESS
    // =========================================================================
    SortEntry* sorted = (SortEntry*)malloc(specie_size * sizeof(SortEntry));
    if (!sorted) {
        free(costs);
        return;
    }

    for (int i = 0; i < specie_size; i++) {
        sorted[i].cost = costs[i];
        sorted[i].index = i;
    }
    qsort(sorted, specie_size, sizeof(SortEntry), compare_by_cost);

    if (enable_logs) {
        printf("[EVOLVE] Best: %.2f, Worst: %.2f\n", 
               sorted[0].cost, sorted[specie_size-1].cost);
    }

    // =========================================================================
    // STEP 3: CREATE NEW POPULATION
    // =========================================================================
    Individual* new_pop = (Individual*)malloc(specie_size * sizeof(Individual));
    if (!new_pop) {
        free(costs);
        free(sorted);
        return;
    }

    for (int i = 0; i < specie_size; i++) {
        Individual_Init(&new_pop[i], total_stations);
    }

    // Copy elites
    int num_elites = (elitism_count < specie_size) ? elitism_count : specie_size;
    for (int e = 0; e < num_elites; e++) {
        Individual_Copy(&new_pop[e], &specie[sorted[e].index]);
        new_pop[e].cached_cost = costs[sorted[e].index];
    }

    // =========================================================================
    // STEP 4: GENERATE OFFSPRING
    // =========================================================================
    int num_offspring = specie_size - num_elites;
    
    // Mating pool = top fraction of population (selection pressure)
    int pool_size = (int)(mating_pool_fraction * specie_size);
    if (pool_size < 2) pool_size = 2;
    if (pool_size > specie_size) pool_size = specie_size;

    for (int i = 0; i < num_offspring; i++) {
        // Select parents from mating pool
        int p1_idx = sorted[RandInt(0, pool_size - 1)].index;
        int p2_idx = sorted[RandInt(0, pool_size - 1)].index;
        Individual* parent1 = &specie[p1_idx];
        Individual* parent2 = &specie[p2_idx];
        Individual* child = &new_pop[num_elites + i];

        // ----- CROSSOVER -----
        int child_size = 0;
        int* child_ring = Slice_Crossover(
            parent1->active_ring, parent1->ring_size,
            parent2->active_ring, parent2->ring_size,
            &child_size, total_stations  // FIX: was 50, now total_stations
        );

        if (child_ring && child_size > 0) {
            memcpy(child->active_ring, child_ring, child_size * sizeof(int));
            child->ring_size = child_size;
            free(child_ring);
        } else {
            Individual_Copy(child, parent1);
        }

        // ----- MUTATION -----
        // Force mutation if child is identical to parent (maintain diversity)
        int identical = 0;
        if (child->ring_size == parent1->ring_size && child->ring_size > 0) {
            if (memcmp(child->active_ring, parent1->active_ring, 
                       child->ring_size * sizeof(int)) == 0) {
                identical = 1;
            }
        }

        if (identical || RandDouble() < mutation_rate) {
            Individual mutated;
            Individual_Init(&mutated, total_stations);  // FIX: was 50, now total_stations
            Mutations(&mutated, add_pct, remove_pct, swap_pct, inv_pct, scr_pct,
                     child, total_stations, dist);
            Individual_Copy(child, &mutated);
            Individual_Free(&mutated);
        }

        // ----- 2-OPT LOCAL SEARCH -----
        if (child->ring_size >= 3) {
            TwoOptImproveAlpha(child, alpha, dist, ranking);
        }

        // ----- EVALUATE -----
        child->cached_cost = Total_Cost_Individual(alpha, child, total_stations, dist, ranking);
    }

    // =========================================================================
    // STEP 5: REPLACE OLD POPULATION
    // =========================================================================
    for (int i = 0; i < specie_size; i++) {
        Individual_Free(&specie[i]);
        specie[i] = new_pop[i];
    }

    // =========================================================================
    // CLEANUP
    // =========================================================================
    free(costs);
    free(sorted);
    free(new_pop);
}

void EvolveSpecie_Sanity(Individual* specie) {
    (void)specie;
}
