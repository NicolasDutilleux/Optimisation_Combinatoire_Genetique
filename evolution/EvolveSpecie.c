// evolution/EvolveSpecie.c
// =============================================================================
// GENETIC EVOLUTION FOR A SINGLE SPECIES
// =============================================================================
//
// This file implements one generation of genetic algorithm evolution:
//   1. Evaluate fitness (cost) of all individuals
//   2. Sort population by fitness (best first)
//   3. Copy elite individuals to next generation
//   4. Generate offspring via crossover and mutation
//   5. Apply local search (2-opt) to improve offspring
//   6. Replace old population with new one
//
// TIMING: When enable_timers is ON, detailed timings are printed for each phase:
//   - Evaluation: Computing fitness of all individuals
//   - Sorting: Ranking individuals by fitness
//   - Crossover: Creating offspring from parents
//   - Mutation: Applying genetic mutations
//   - TwoOpt: Local search optimization
//   - Replace: Swapping old population with new
//
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
// HIGH-RESOLUTION TIMER (Windows QueryPerformanceCounter)
// =============================================================================
// Uses CPU timestamp counter for microsecond precision.
// Much more accurate than clock() for short operations.

typedef struct {
    LARGE_INTEGER start;
    LARGE_INTEGER freq;
} PerfTimer;

static void perf_start(PerfTimer* t) {
    QueryPerformanceFrequency(&t->freq);
    QueryPerformanceCounter(&t->start);
}

static double perf_ms(PerfTimer* t) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)(now.QuadPart - t->start.QuadPart) * 1000.0 / (double)t->freq.QuadPart;
}

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
    // Validation
    if (!specie || specie_size <= 0) return;

    // =========================================================================
    // TIMING VARIABLES
    // =========================================================================
    PerfTimer timer_total, timer_step;
    double time_eval = 0, time_sort = 0, time_crossover = 0;
    double time_mutation = 0, time_twoopt = 0, time_replace = 0;
    
    if (enable_timers) perf_start(&timer_total);

    // =========================================================================
    // STEP 1: EVALUATE ALL INDIVIDUALS (compute fitness/cost)
    // =========================================================================
    // This is often the most expensive step. Uses cached costs when available.
    // Optimization: Cost functions use stack allocation for small arrays.
    
    if (enable_timers) perf_start(&timer_step);
    
    double* costs = (double*)malloc(specie_size * sizeof(double));
    if (!costs) return;

    for (int i = 0; i < specie_size; i++) {
        // Initialize if needed
        if (!specie[i].active_ring) {
            Individual_Init(&specie[i], 50);
        }
        
        // Use cached cost if valid (OPTIMIZATION: avoids recomputation)
        if (specie[i].cached_cost < 1e17) {
            costs[i] = specie[i].cached_cost;
        } else {
            costs[i] = Total_Cost_Individual(alpha, &specie[i], total_stations, dist, ranking);
            specie[i].cached_cost = costs[i];
        }
    }

    if (enable_timers) time_eval = perf_ms(&timer_step);

    // =========================================================================
    // STEP 2: SORT BY FITNESS (best = lowest cost first)
    // =========================================================================
    // Uses qsort with index tracking to avoid moving Individual structs.
    
    if (enable_timers) perf_start(&timer_step);
    
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

    if (enable_timers) time_sort = perf_ms(&timer_step);

    if (enable_logs) {
        printf("[EVOLVE] Best: %.2f, Worst: %.2f\n", 
               sorted[0].cost, sorted[specie_size-1].cost);
    }

    // =========================================================================
    // STEP 3: CREATE NEW POPULATION BUFFER
    // =========================================================================
    Individual* new_pop = (Individual*)malloc(specie_size * sizeof(Individual));
    if (!new_pop) {
        free(costs);
        free(sorted);
        return;
    }

    for (int i = 0; i < specie_size; i++) {
        Individual_Init(&new_pop[i], 50);
    }

    // Copy elites (best individuals survive unchanged)
    int num_elites = (elitism_count < specie_size) ? elitism_count : specie_size;
    for (int e = 0; e < num_elites; e++) {
        Individual_Copy(&new_pop[e], &specie[sorted[e].index]);
        new_pop[e].cached_cost = costs[sorted[e].index];
    }

    // =========================================================================
    // STEP 4: GENERATE OFFSPRING (crossover + mutation + local search)
    // =========================================================================
    int num_offspring = specie_size - num_elites;
    
    // Mating pool = top fraction of population (selection pressure)
    int pool_size = (int)(mating_pool_fraction * specie_size);
    if (pool_size < 2) pool_size = 2;
    if (pool_size > specie_size) pool_size = specie_size;

    // Accumulators for detailed timing
    double acc_crossover = 0, acc_mutation = 0, acc_twoopt = 0, acc_eval = 0;

    for (int i = 0; i < num_offspring; i++) {
        // Select parents from mating pool (tournament selection)
        int p1_idx = sorted[RandInt(0, pool_size - 1)].index;
        int p2_idx = sorted[RandInt(0, pool_size - 1)].index;
        Individual* parent1 = &specie[p1_idx];
        Individual* parent2 = &specie[p2_idx];
        Individual* child = &new_pop[num_elites + i];

        // ----- CROSSOVER -----
        // Slice crossover: take a random slice from parent1, fill rest from parent2
        if (enable_timers) perf_start(&timer_step);
        
        int child_size = 0;
        int* child_ring = Slice_Crossover(
            parent1->active_ring, parent1->ring_size,
            parent2->active_ring, parent2->ring_size,
            &child_size, 50
        );

        if (child_ring && child_size > 0) {
            // Resize child buffer if needed
            if (child_size > child->ring_capacity) {
                int* tmp = (int*)realloc(child->active_ring, child_size * sizeof(int));
                if (tmp) {
                    child->active_ring = tmp;
                    child->ring_capacity = child_size;
                }
            }
            memcpy(child->active_ring, child_ring, child_size * sizeof(int));
            child->ring_size = child_size;
            free(child_ring);
        }
        
        if (enable_timers) acc_crossover += perf_ms(&timer_step);

        // ----- MUTATION -----
        // Force mutation if child is identical to parent (maintain diversity)
        if (enable_timers) perf_start(&timer_step);
        
        int identical = 0;
        if (child->ring_size == parent1->ring_size && child->ring_size > 0) {
            if (memcmp(child->active_ring, parent1->active_ring, 
                       child->ring_size * sizeof(int)) == 0) {
                identical = 1;
            }
        }

        if (identical || RandDouble() < mutation_rate) {
            Individual mutated;
            Individual_Init(&mutated, 50);
            Mutations(&mutated, add_pct, remove_pct, swap_pct, inv_pct, scr_pct,
                     child, total_stations, dist);
            Individual_Copy(child, &mutated);
            Individual_Free(&mutated);
        }
        
        if (enable_timers) acc_mutation += perf_ms(&timer_step);

        // ----- LOCAL SEARCH (2-OPT) -----
        // Improves ring by reversing segments that reduce total distance
        if (enable_timers) perf_start(&timer_step);
        
        if (child->ring_size >= 3) {
            TwoOptImproveAlpha(child, alpha, dist, ranking);
        }
        
        if (enable_timers) acc_twoopt += perf_ms(&timer_step);

        // ----- EVALUATE CHILD -----
        if (enable_timers) perf_start(&timer_step);
        
        child->cached_cost = Total_Cost_Individual(alpha, child, total_stations, dist, ranking);
        
        if (enable_timers) acc_eval += perf_ms(&timer_step);
    }

    // Store accumulated times
    time_crossover = acc_crossover;
    time_mutation = acc_mutation;
    time_twoopt = acc_twoopt;

    // =========================================================================
    // STEP 5: REPLACE OLD POPULATION
    // =========================================================================
    if (enable_timers) perf_start(&timer_step);

    for (int i = 0; i < specie_size; i++) {
        Individual_Free(&specie[i]);
        specie[i] = new_pop[i];
    }

    if (enable_timers) time_replace = perf_ms(&timer_step);

    // =========================================================================
    // CLEANUP
    // =========================================================================
    free(costs);
    free(sorted);
    free(new_pop);  // Note: Individual contents moved to specie[], don't free them

    // =========================================================================
    // TIMING REPORT
    // =========================================================================
    if (enable_timers) {
        double total = perf_ms(&timer_total);
        printf("[EVOLVE TIMERS]\n");
        printf("  Evaluation:  %6.2f ms\n", time_eval);
        printf("  Sorting:     %6.2f ms\n", time_sort);
        printf("  Crossover:   %6.2f ms (total for %d offspring)\n", time_crossover, num_offspring);
        printf("  Mutation:    %6.2f ms\n", time_mutation);
        printf("  2-Opt:       %6.2f ms\n", time_twoopt);
        printf("  Replace:     %6.2f ms\n", time_replace);
        printf("  TOTAL:       %6.2f ms\n", total);
    }
}

// Sanity check function (for testing)
void EvolveSpecie_Sanity(Individual* specie) {
    (void)specie;
}
