// main.c - Genetic Algorithm for Ring Optimization
// Entry point and orchestration

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "core\Node.h"
#include "core\Individual.h"
#include "generation\PopulationInit.h"
#include "utils\Distance.h"
#include "utils\Random.h"
#include "utils\ThreadPool.h"
#include "evolution\EvolveSpecie.h"
#include "cost\Cost.h"
#include "genetic\Selection.h"
#include "utils\FileIO.h"
#include "utils\hierarchy_and_print_utils.h"
#include "utils\main_helpers.h"

// =============================================================================
// HIGH-RESOLUTION TIMER
// =============================================================================
typedef struct {
    LARGE_INTEGER start;
    LARGE_INTEGER freq;
} Timer;

static void timer_start(Timer* t) {
    QueryPerformanceFrequency(&t->freq);
    QueryPerformanceCounter(&t->start);
}

static double timer_ms(Timer* t) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)(now.QuadPart - t->start.QuadPart) * 1000.0 / (double)t->freq.QuadPart;
}

// =============================================================================
// MAIN PROGRAM
// =============================================================================
int main(int argc, char** argv)
{
    printf("============================================\n");
    printf("  GENETIC ALGORITHM - RING OPTIMIZATION\n");
    printf("============================================\n\n");

    Timer total_timer;
    timer_start(&total_timer);

    // -------------------------------------------------------------------------
    // DEFAULT PARAMETERS
    // -------------------------------------------------------------------------
    int max_generations = 10000;
    int log_interval = 50;
    int verbose = 0;
    
    int num_species = 100;
    int pop_size = 50;
    double mutation_rate = 0.25;
    int alpha = 3;
    int elitism = 0;
    
    int add_pct = 15, remove_pct = 10, swap_pct = 15;
    int inv_pct = 5, scr_pct = 5;
    
    int num_threads = 0;
    int enable_logs = 0;
    int enable_timers = 0;

    // -------------------------------------------------------------------------
    // PARSE COMMAND LINE
    // -------------------------------------------------------------------------
    parse_args(argc, argv, &max_generations, &log_interval, &num_species,
               &pop_size, &num_threads, &verbose, &enable_logs, &enable_timers);

    // Auto-detect thread count
    if (num_threads <= 0) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        num_threads = si.dwNumberOfProcessors;
        if (num_threads > num_species) num_threads = num_species;
    }

    printf("[CONFIG]\n");
    printf("  Generations: %d\n", max_generations);
    printf("  Species:     %d\n", num_species);
    printf("  Population:  %d per species\n", pop_size);
    printf("  Threads:     %d\n", num_threads);
    printf("  Logs:        %s\n", enable_logs ? "ON" : "OFF");
    printf("  Timers:      %s\n\n", enable_timers ? "ON" : "OFF");

    // -------------------------------------------------------------------------
    // STEP 1: LOAD DATASET
    // -------------------------------------------------------------------------
    printf("[STEP 1] Loading dataset...\n");
    
    int total_stations = 0;
    Node* nodes = readDataset("data\\51\\51_data.txt", &total_stations);
    
    if (!nodes || total_stations == 0) {
        fprintf(stderr, "ERROR: Failed to load dataset\n");
        return 1;
    }
    printf("         %d stations loaded\n\n", total_stations);

    // -------------------------------------------------------------------------
    // STEP 2: COMPUTE DISTANCE MATRIX
    // -------------------------------------------------------------------------
    printf("[STEP 2] Computing distance matrix...\n");
    
    Timer t;
    timer_start(&t);
    double** dist = Compute_Distances_2DVector(nodes, total_stations);
    
    if (!dist) {
        fprintf(stderr, "ERROR: Distance computation failed\n");
        free(nodes);
        return 1;
    }
    printf("         Done (%.1f ms)\n\n", timer_ms(&t));

    // -------------------------------------------------------------------------
    // STEP 3: COMPUTE DISTANCE RANKING
    // -------------------------------------------------------------------------
    printf("[STEP 3] Computing distance ranking...\n");
    
    timer_start(&t);
    int** ranking = Distance_Ranking_2DVector(dist, total_stations, total_stations);
    
    if (!ranking) {
        fprintf(stderr, "ERROR: Ranking computation failed\n");
        Free_2DArray_Double(dist, total_stations);
        free(nodes);
        return 1;
    }
    printf("         Done (%.1f ms)\n\n", timer_ms(&t));

    // -------------------------------------------------------------------------
    // STEP 4: INITIALIZE POPULATION
    // -------------------------------------------------------------------------
    printf("[STEP 4] Initializing population...\n");
    
    RNG_Init();
    timer_start(&t);
    
    int species_count = 0;
    Individual** species = Random_Generation(nodes, total_stations, 
                                              num_species, pop_size, &species_count);
    
    if (!species) {
        fprintf(stderr, "ERROR: Population initialization failed\n");
        Free_2DArray_Int(ranking, total_stations);
        Free_2DArray_Double(dist, total_stations);
        free(nodes);
        return 1;
    }
    printf("         %d species x %d individuals (%.1f ms)\n\n", 
           num_species, pop_size, timer_ms(&t));

    // -------------------------------------------------------------------------
    // STEP 5: CREATE THREAD POOL
    // -------------------------------------------------------------------------
    printf("[STEP 5] Creating thread pool...\n");
    
    int pool_ok = ThreadPool_Init(num_threads);
    if (!pool_ok) {
        fprintf(stderr, "WARNING: Thread pool failed, using single thread\n");
        num_threads = 0;
    }
    printf("         %d worker threads ready\n\n", num_threads);

    // Pre-allocate task array
    EvolveTask* tasks = (EvolveTask*)malloc(num_species * sizeof(EvolveTask));

    // -------------------------------------------------------------------------
    // STEP 6: EVOLUTION LOOP
    // -------------------------------------------------------------------------
    printf("[STEP 6] Starting evolution...\n");
    printf("============================================\n\n");

    double best_cost = 1e18;
    int stagnation = 0;
    int stagnation_limit = 50;

    for (int gen = 0; gen < max_generations; gen++) {
        
        Timer gen_timer;
        timer_start(&gen_timer);

        // Build task list
        for (int s = 0; s < num_species; s++) {
            tasks[s].specie = species[s];
            tasks[s].pop_size = pop_size;
            tasks[s].dist = dist;
            tasks[s].ranking = ranking;
            tasks[s].nodes = nodes;
            tasks[s].total_stations = total_stations;
            tasks[s].alpha = alpha;
            tasks[s].mutation_rate = mutation_rate;
            tasks[s].elitism = elitism;
            tasks[s].add_pct = add_pct;
            tasks[s].remove_pct = remove_pct;
            tasks[s].swap_pct = swap_pct;
            tasks[s].inv_pct = inv_pct;
            tasks[s].scr_pct = scr_pct;
        }

        // Execute evolution (parallel or serial)
        if (num_threads > 0) {
            ThreadPool_Run(tasks, num_species, enable_logs, enable_timers);
        } else {
            for (int s = 0; s < num_species; s++) {
                EvolveTask* tk = &tasks[s];
                EvolveSpecie(
                    tk->specie, tk->pop_size,
                    (const double**)tk->dist, (const int**)tk->ranking,
                    tk->nodes, tk->total_stations,
                    tk->alpha, tk->total_stations,
                    tk->mutation_rate, tk->elitism,
                    tk->add_pct, tk->remove_pct, tk->swap_pct,
                    tk->inv_pct, tk->scr_pct, 0.5,
                    enable_logs, enable_timers
                );
            }
        }

        // Progress report
        if (gen % log_interval == 0) {
            evaluate_and_report(
                gen, num_species, pop_size, dist, ranking,
                nodes, total_stations, alpha, species,
                &best_cost, &stagnation, &mutation_rate,
                enable_logs, enable_timers
            );
            
            if (stagnation > stagnation_limit) {
                printf("\n[STOP] Stagnation after %d generations\n", stagnation);
                break;
            }
        }

        // Generation timer
        if (enable_timers) {
            printf("[TIMER] Gen %d: %.1f ms\n", gen, timer_ms(&gen_timer));
        }

        // Progress dots
        if (!enable_logs && !enable_timers && gen % 10 == 0) {
            printf(".");
            fflush(stdout);
            if (gen % 100 == 0 && gen > 0) {
                printf(" [gen %d]\n", gen);
            }
        }
    }

    printf("\n\n");

    // -------------------------------------------------------------------------
    // STEP 7: FINAL RESULTS
    // -------------------------------------------------------------------------
    printf("============================================\n");
    printf("[STEP 7] Final evaluation\n\n");

    double final_best = 1e18;
    int best_species = 0, best_idx = 0;

    for (int s = 0; s < num_species; s++) {
        double* costs = Total_Cost_Specie(alpha, species[s], pop_size,
                                          total_stations,
                                          (const double**)dist,
                                          (const int**)ranking);
        if (costs) {
            int idx = Select_Best(costs, pop_size);
            if (costs[idx] < final_best) {
                final_best = costs[idx];
                best_species = s;
                best_idx = idx;
            }
            free(costs);
        }
    }

    printf("  BEST SOLUTION\n");
    printf("  -------------\n");
    printf("  Cost:    %.2f\n", final_best);
    printf("  Species: %d\n", best_species);
    printf("  Index:   %d\n", best_idx);
    
    Individual* best = &species[best_species][best_idx];
    printf("  Ring:    ");
    for (int i = 0; i < best->ring_size && i < 15; i++) {
        printf("%d ", best->active_ring[i]);
    }
    if (best->ring_size > 15) printf("...");
    printf("\n  Size:    %d nodes\n\n", best->ring_size);

    // -------------------------------------------------------------------------
    // STEP 8: CLEANUP
    // -------------------------------------------------------------------------
    printf("[STEP 8] Cleanup...\n");

    ThreadPool_Destroy();
    free(tasks);
    Free_Population(species, num_species, pop_size);
    Free_2DArray_Int(ranking, total_stations);
    Free_2DArray_Double(dist, total_stations);
    free(nodes);

    printf("         Done\n\n");

    // -------------------------------------------------------------------------
    // SUMMARY
    // -------------------------------------------------------------------------
    printf("============================================\n");
    printf("  TOTAL TIME: %.1f seconds\n", timer_ms(&total_timer) / 1000.0);
    printf("============================================\n");

    return 0;
}
