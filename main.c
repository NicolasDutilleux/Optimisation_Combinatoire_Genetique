// main.c - Genetic Algorithm for Ring Optimization
// Entry point and orchestration

#define _CRT_SECURE_NO_WARNINGS  // Disable MSVC security warnings for scanf/sprintf

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "core\Node.h"
#include "core\Individual.h"
#include "generation\PopulationInit.h"
#include "utils\Distance.h"
#include "utils\Random.h"
#include "utils\ThreadPool.h"
#include "utils\Visualize.h"
#include "evolution\EvolveSpecie.h"
#include "cost\Cost.h"
#include "genetic\Selection.h"
#include "utils\FileIO.h"
#include "utils\hierarchy_and_print_utils.h"
#include "utils\main_helpers.h"
#include "utils\OutputWriter.h"

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

static double timer_seconds(Timer* t) {
    return timer_ms(t) / 1000.0;
}

// =============================================================================
// MAIN PROGRAM
// =============================================================================
int main(int argc, char** argv)
{
    printf("============================================\n");
    printf("  GENETIC ALGORITHM - RING OPTIMIZATION\n");
    printf("============================================\n\n");

    // -------------------------------------------------------------------------
    // USER INPUT: Time limit, Dataset, Alpha
    // -------------------------------------------------------------------------
    double time_limit_seconds;
    int alpha;
    int dataset_num;
    char dataset_path[256];

    printf("Time limit in seconds (e.g., 58): ");
    if (scanf("%lf", &time_limit_seconds) != 1 || time_limit_seconds <= 0) {
        fprintf(stderr, "ERROR: Invalid time limit\n");
        return 1;
    }

    printf("Available datasets: 51, 100, 127, 225\n");
    printf("Dataset number: ");
    if (scanf("%d", &dataset_num) != 1) {
        fprintf(stderr, "ERROR: Invalid dataset number\n");
        return 1;
    }

    printf("Alpha (3, 5, 7, 9): ");
    if (scanf("%d", &alpha) != 1 || (alpha != 3 && alpha != 5 && alpha != 7 && alpha != 9)) {
        fprintf(stderr, "ERROR: Alpha must be 3, 5, 7, or 9\n");
        return 1;
    }

    // Build dataset path: data\{num}\{num}_data.txt
    sprintf(dataset_path, "data\\%d\\%d_data.txt", dataset_num, dataset_num);
    
    printf("\n");

    // Start the timer AFTER user input
    Timer total_timer;
    timer_start(&total_timer);

    // -------------------------------------------------------------------------
    // DEFAULT PARAMETERS
    // -------------------------------------------------------------------------
    int max_generations = 1000000;  // Very high - will stop by time limit
    int log_interval = 150;
    int verbose = 0;
    
    int num_species = 30;
    int pop_size = 200;
    double mutation_rate = 0.30;
    
    // ÉLITISME : 5% des meilleurs survivent
    int elitism = (pop_size * 5) / 100;
    if (elitism < 1) elitism = 1;
    
    int add_pct = 15, remove_pct = 10, swap_pct = 15;
    int inv_pct = 5, scr_pct = 5;
    
    int num_threads = 0;
    int enable_logs = 0;
    int enable_timers = 0;

    // -------------------------------------------------------------------------
    // PARSE COMMAND LINE (optional overrides)
    // -------------------------------------------------------------------------
    parse_args(argc, argv, &max_generations, &log_interval, &num_species,
               &pop_size, &num_threads, &verbose, &enable_logs, &enable_timers);

    // Recalculate elitism after pop_size might have changed
    elitism = (pop_size * 5) / 100;
    if (elitism < 1) elitism = 1;

    // Auto-detect thread count
    if (num_threads <= 0) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        num_threads = si.dwNumberOfProcessors;
        if (num_threads > num_species) num_threads = num_species;
    }

    printf("[CONFIG]\n");
    printf("  Time limit:  %.1f seconds\n", time_limit_seconds);
    printf("  Dataset:     %s\n", dataset_path);
    printf("  Alpha:       %d\n", alpha);
    printf("  Species:     %d\n", num_species);
    printf("  Population:  %d per species\n", pop_size);
    printf("  Elitism:     %d (top individuals preserved)\n", elitism);
    printf("  Threads:     %d\n\n", num_threads);

    // -------------------------------------------------------------------------
    // STEP 1: LOAD DATASET
    // -------------------------------------------------------------------------
    printf("[STEP 1] Loading dataset...\n");
    
    int total_stations = 0;
    Node* nodes = readDataset(dataset_path, &total_stations);
    
    if (!nodes || total_stations == 0) {
        fprintf(stderr, "ERROR: Failed to load dataset '%s'\n", dataset_path);
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
    printf("         %d species x %d individuals (%.1f ms)\n", 
           num_species, pop_size, timer_ms(&t));

    // -------------------------------------------------------------------------
    // STEP 4b: APPLY 2-OPT TO PART OF INITIAL POPULATION
    // -------------------------------------------------------------------------
    printf("         Applying 2-Opt (exhaustive) to 20%% of species...\n");
    timer_start(&t);
    
    Apply_TwoOpt_To_Population(species, num_species, pop_size, alpha,
                               (const double**)dist, (const int**)ranking);
    
    printf("         Done (%.1f ms)\n\n", timer_ms(&t));

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
    // STEP 6: EVOLUTION LOOP (with time limit)
    // -------------------------------------------------------------------------
    printf("[STEP 6] Starting evolution (time limit: %.1f s)...\n", time_limit_seconds);
    printf("============================================\n\n");

    double best_cost = 1e18;
    int stagnation = 0;
    int stagnation_limit = 50;
    int gen = 0;
    int visualization_done = 0;  // Flag pour ne visualiser qu'une seule fois

    // Pour détecter les espèces avec le même score
    double* species_best_costs = (double*)malloc(num_species * sizeof(double));
    int* species_stagnation = (int*)calloc(num_species, sizeof(int));  // Compteur de stagnation par espèce

    // Evolution loop - stops when time limit is reached
    while (timer_seconds(&total_timer) < time_limit_seconds) {
        
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

        // =====================================================================
        // DUPLICATE SPECIES DETECTION & RESET
        // =====================================================================
        // Toutes les 10 générations, vérifier si des espèces ont convergé vers
        // le même coût et les réinitialiser pour maintenir la diversité
        if (gen > 0 && gen % 10 == 0) {
            // Calculer le meilleur coût de chaque espèce
            for (int s = 0; s < num_species; s++) {
                double* costs = Total_Cost_Specie(alpha, species[s], pop_size,
                                                  total_stations,
                                                  (const double**)dist,
                                                  (const int**)ranking);
                if (costs) {
                    int idx = Select_Best(costs, pop_size);
                    double new_best = costs[idx];
                    
                    // Vérifier si l'espèce stagne
                    if (fabs(new_best - species_best_costs[s]) < 0.01) {
                        species_stagnation[s]++;
                    } else {
                        species_stagnation[s] = 0;
                    }
                    species_best_costs[s] = new_best;
                    free(costs);
                }
            }
            
            // Trouver les espèces avec des coûts EXACTEMENT ÉGAUX
            for (int s1 = 0; s1 < num_species; s1++) {
                for (int s2 = s1 + 1; s2 < num_species; s2++) {
                    double diff = fabs(species_best_costs[s1] - species_best_costs[s2]);
                    
                    // Coûts ÉGAUX (tolérance pour erreurs floating point: < 0.01)
                    // ET les deux espèces stagnent depuis > 5 vérifications
                    if (diff < 0.01 && 
                        species_stagnation[s1] > 5 && species_stagnation[s2] > 5) {
                        
                        // Réinitialiser s2 (garder s1)
                        int to_reset = s2;
                        
                        printf("[DIVERSITY] Species %d and %d have EQUAL cost (%.2f) - resetting species %d\n",
                               s1, s2, species_best_costs[s1], to_reset);
                        
                        // Réinitialiser tous les individus de cette espèce
                        for (int i = 0; i < pop_size; i++) {
                            Individual_Free(&species[to_reset][i]);
                            Individual_Init(&species[to_reset][i], total_stations);
                            
                            // Créer un nouveau ring aléatoire
                            int ring_size = RandInt(2, total_stations);
                            int* all_ids = (int*)malloc(total_stations * sizeof(int));
                            for (int j = 0; j < total_stations; j++) all_ids[j] = j + 1;
                            for (int j = total_stations - 1; j > 0; j--) {
                                int k = RandInt(0, j);
                                int tmp = all_ids[j]; all_ids[j] = all_ids[k]; all_ids[k] = tmp;
                            }
                            
                            species[to_reset][i].active_ring[0] = 1;  // Dépôt
                            species[to_reset][i].ring_size = 1;
                            int added = 0;
                            for (int k = 0; k < total_stations && added < ring_size - 1; k++) {
                                if (all_ids[k] != 1) {
                                    species[to_reset][i].active_ring[species[to_reset][i].ring_size++] = all_ids[k];
                                    added++;
                                }
                            }
                            free(all_ids);
                            species[to_reset][i].cached_cost = 1e18;
                        }
                        
                        species_stagnation[to_reset] = 0;
                        species_best_costs[to_reset] = 1e18;
                        
                        // Ne réinitialiser qu'une espèce par vérification
                        break;
                    }
                }
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
            
            // Show remaining time
            double elapsed = timer_seconds(&total_timer);
            double remaining = time_limit_seconds - elapsed;
            printf("  Time: %.1f s elapsed, %.1f s remaining\n\n", elapsed, remaining);
            
            // Visualize ONCE when stagnation reaches limit for the first time
            if (stagnation >= stagnation_limit && !visualization_done) {
                printf("\n[STAGNATION] Reached %d - generating visualization...\n", stagnation);
                
                // Find the best individual for visualization
                double viz_best_cost = 1e18;
                int viz_best_species = 0, viz_best_idx = 0;
                
                for (int s = 0; s < num_species; s++) {
                    double* costs = Total_Cost_Specie(alpha, species[s], pop_size,
                                                      total_stations,
                                                      (const double**)dist,
                                                      (const int**)ranking);
                    if (costs) {
                        int idx = Select_Best(costs, pop_size);
                        if (costs[idx] < viz_best_cost) {
                            viz_best_cost = costs[idx];
                            viz_best_species = s;
                            viz_best_idx = idx;
                        }
                        free(costs);
                    }
                }
                
                // Generate visualization (only once)
                Visualize_Ring(&species[viz_best_species][viz_best_idx], nodes, 
                              total_stations, viz_best_cost, alpha, gen);
                
                visualization_done = 1;  // Ne plus visualiser après
                
                // NE PAS remettre stagnation à 0 !
                // Le mécanisme d'augmentation de mutation doit continuer
                printf("[VISUALIZATION] Done - mutation rate adaptation continues\n\n");
            }
        }

        gen++;
    }

    // Libérer les tableaux de suivi
    free(species_best_costs);
    free(species_stagnation);

    printf("\n[TIME LIMIT] Stopped after %.1f seconds (%d generations)\n\n", 
           timer_seconds(&total_timer), gen);

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
    // FINAL VISUALIZATION (at end of timer)
    // -------------------------------------------------------------------------
    printf("[FINAL VISUALIZATION] Generating final result image...\n");
    Visualize_Ring(best, nodes, total_stations, final_best, alpha, gen);

    // -------------------------------------------------------------------------
    // WRITE SOLUTION TO FILE
    // -------------------------------------------------------------------------
    printf("\n[WRITING SOLUTION] Creating Genetic_Solution.txt...\n");
    WriteSolutionToFile(best, total_stations, alpha, final_best, 
                       (const int**)ranking);

    // -------------------------------------------------------------------------
    // STEP 8: CLEANUP
    // -------------------------------------------------------------------------
    printf("\n[STEP 8] Cleanup...\n");

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
    printf("  DATASET: %d stations (alpha=%d)\n", total_stations, alpha);
    printf("  GENERATIONS: %d\n", gen);
    printf("  BEST COST: %.2f\n", final_best);
    printf("  TOTAL TIME: %.1f seconds\n", timer_seconds(&total_timer));
    printf("============================================\n");

    return 0;
}
