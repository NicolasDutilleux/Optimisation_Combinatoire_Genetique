// utils/main_helpers.c
// Implementation of helper functions for main program

#include "utils\main_helpers.h"
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "utils\hierarchy_and_print_utils.h"
#include "utils\FileIO.h"
#include "utils\Distance.h"
#include "cost\Cost.h"
#include "genetic\Selection.h"

// =============================================================================
// INTERNAL FUNCTIONS
// =============================================================================

// Comparison for sorting species by cost (ascending)
static int compare_species_cost(const void* a, const void* b) {
    double da = ((const SpeciesCostLocal*)a)->cost;
    double db = ((const SpeciesCostLocal*)b)->cost;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

void print_now(const char* label) {
    time_t t = time(NULL);
    struct tm tm_info;
    
#ifdef _WIN32
    localtime_s(&tm_info, &t);
#else
    localtime_r(&t, &tm_info);
#endif

    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_info);
    printf("[%s] %s\n", buffer, label);
}

void parse_args(
    int argc, char** argv,
    int* max_generations,
    int* LOG_INTERVAL,
    int* NUM_SPECIES,
    int* POP_SIZE,
    int* requested_workers,
    int* verbose,
    int* enable_logs,
    int* enable_timers)
{
    for (int i = 1; i < argc; ++i) {
        // Flag: verbose
        if (strcmp(argv[i], "-v") == 0) {
            *verbose = 1;
        }
        // Flag: generations
        else if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) {
            *max_generations = atoi(argv[++i]);
        }
        // Flag: species count
        else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            *NUM_SPECIES = atoi(argv[++i]);
        }
        // Flag: population size
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            *POP_SIZE = atoi(argv[++i]);
        }
        // Flag: log interval
        else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            *LOG_INTERVAL = atoi(argv[++i]);
        }
        // Flag: workers (kept for compatibility but not used)
        else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            *requested_workers = atoi(argv[++i]);
            if (*requested_workers < 0) *requested_workers = 0;
        }
        // Flag: enable logs
        else if (strcmp(argv[i], "--logs") == 0) {
            if (enable_logs) *enable_logs = 1;
        }
        // Flag: enable timers
        else if (strcmp(argv[i], "--timers") == 0) {
            if (enable_timers) *enable_timers = 1;
        }
        // Positional: number = generations
        else if (argv[i][0] >= '0' && argv[i][0] <= '9') {
            *max_generations = atoi(argv[i]);
        }
    }
}

void evaluate_and_report(
    int gen,
    int NUM_SPECIES,
    int POP_SIZE,
    double** dist,
    int** dist_ranking,
    Node* node_vector,
    int total_stations,
    int ALPHA,
    Individual** species,
    double* old_best_ptr,
    int* stagnation_count_ptr,
    double* MUTATION_RATE_ptr,
    int enable_logs,
    int enable_timers)
{
    // -------------------------------------------------------------------------
    // Allocate storage for species costs
    // -------------------------------------------------------------------------
    SpeciesCostLocal* species_costs = (SpeciesCostLocal*)malloc(
        NUM_SPECIES * sizeof(SpeciesCostLocal)
    );
    if (!species_costs) return;

    // -------------------------------------------------------------------------
    // Evaluate each species
    // -------------------------------------------------------------------------
    clock_t start = 0;
    if (enable_timers) start = clock();

    for (int s = 0; s < NUM_SPECIES; ++s) {
        double* costs = Total_Cost_Specie(
            ALPHA, species[s], POP_SIZE, total_stations,
            (const double**)dist, (const int**)dist_ranking
        );

        if (!costs) {
            species_costs[s].cost = 1e18;
            species_costs[s].best_idx = -1;
        } else {
            int best_idx = Select_Best(costs, POP_SIZE);
            species_costs[s].cost = costs[best_idx];
            species_costs[s].best_idx = best_idx;
            free(costs);
        }
        species_costs[s].species_id = s;
    }

    if (enable_timers) {
        clock_t end = clock();
        double elapsed_ms = ((double)(end - start)) * 1000.0 / CLOCKS_PER_SEC;
        printf("  [TIMER] Evaluation: %.1f ms\n", elapsed_ms);
    }

    // -------------------------------------------------------------------------
    // Sort species by cost
    // -------------------------------------------------------------------------
    qsort(species_costs, NUM_SPECIES, sizeof(SpeciesCostLocal), compare_species_cost);

    // -------------------------------------------------------------------------
    // Print progress report
    // -------------------------------------------------------------------------
    printf("\n");
    printf("  [GENERATION %d]\n", gen);
    printf("  ---------------\n");
    
    // Top 5 species
    int top_count = (5 < NUM_SPECIES) ? 5 : NUM_SPECIES;
    printf("  Top %d species:\n", top_count);
    for (int i = 0; i < top_count; ++i) {
        printf("    %d. Species %3d: cost = %.2f\n", 
               i + 1, species_costs[i].species_id, species_costs[i].cost);
    }

    // Best individual details
    double best_cost = species_costs[0].cost;
    int best_species = species_costs[0].species_id;
    int best_idx = species_costs[0].best_idx;

    printf("\n  Best: cost=%.2f (species %d, individual %d)\n", 
           best_cost, best_species, best_idx);

    // Show ring if logs enabled
    if (enable_logs && best_idx >= 0) {
        Individual* best = &species[best_species][best_idx];
        printf("  Ring: ");
        for (int k = 0; k < best->ring_size && k < 15; ++k) {
            printf("%d ", best->active_ring[k]);
        }
        if (best->ring_size > 15) printf("...");
        printf("(size=%d)\n", best->ring_size);
    }

    // -------------------------------------------------------------------------
    // Generate visualization (only if logs enabled and improvement found)
    // -------------------------------------------------------------------------
    if (enable_logs && best_cost < *old_best_ptr && best_idx >= 0) {
        Individual* best = &species[best_species][best_idx];
        int assign_len = 0;
        int* assign_pairs = BuildAssignmentPairs(
            best->active_ring, best->ring_size,
            (const int**)dist_ranking, total_stations, &assign_len
        );
        PlotIndividualSVG_C(
            best->active_ring, best->ring_size,
            node_vector, total_stations, gen, assign_pairs, assign_len
        );
        if (assign_pairs) free(assign_pairs);
    }

    // -------------------------------------------------------------------------
    // Update best and stagnation
    // -------------------------------------------------------------------------
    if (best_cost < *old_best_ptr) {
        printf("  >>> IMPROVEMENT: %.2f -> %.2f\n", *old_best_ptr, best_cost);
        *old_best_ptr = best_cost;
        *stagnation_count_ptr = 0;
        
        // Reset mutation rate on improvement
        if (*MUTATION_RATE_ptr > 0.30) {
            *MUTATION_RATE_ptr = 0.30;
            printf("  Mutation rate reset to %.3f\n", *MUTATION_RATE_ptr);
        }
    } else {
        (*stagnation_count_ptr)++;
        printf("  No improvement (stagnation: %d)\n", *stagnation_count_ptr);

        // Increase mutation rate progressively based on stagnation level
        if ((*stagnation_count_ptr) % 5 == 0) {
            double new_rate;
            
            if (*stagnation_count_ptr < 20) {
                // Phase 1: Augmentation modérée (30% -> 50%)
                new_rate = (*MUTATION_RATE_ptr) * 1.20;
                if (new_rate > 0.50) new_rate = 0.50;
            } else if (*stagnation_count_ptr < 50) {
                // Phase 2: Augmentation forte (50% -> 70%)
                new_rate = (*MUTATION_RATE_ptr) * 1.15;
                if (new_rate > 0.70) new_rate = 0.70;
            } else {
                // Phase 3: Mode exploration maximale (70% -> 90%)
                new_rate = (*MUTATION_RATE_ptr) * 1.10;
                if (new_rate > 0.90) new_rate = 0.90;
            }
            
            if (new_rate > *MUTATION_RATE_ptr) {
                *MUTATION_RATE_ptr = new_rate;
                printf("  Mutation rate increased to %.3f\n", *MUTATION_RATE_ptr);
            }
        }
    }

    printf("\n");

    // -------------------------------------------------------------------------
    // Cleanup
    // -------------------------------------------------------------------------
    free(species_costs);
}
