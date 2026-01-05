// main.c - Pure C version (refactored for readability)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <process.h>

#include "core\Node.h"
#include "core\Individual.h"
#include "generation\PopulationInit.h"
#include "utils\Distance.h"
#include "utils\Random.h"
#include "evolution\EvolveSpecie.h"
#include "cost\Cost.h"
#include "genetic\Selection.h"
#include "utils\FileIO.h"
#include "utils\hierarchy_and_print_utils.h"
#include "utils\main_helpers.h"

// global critical section to protect non-thread-safe calls
static CRITICAL_SECTION evolve_cs;

// Worker parameter struct for thread per species
typedef struct {
    Individual* ind; // pointer to the species individual (species[s])
    int POP_SIZE;
    double** dist;
    int** dist_ranking;
    Node* node_vector;
    int total_stations;
    int ALPHA;
    double MUTATION_RATE;
    int ELITISM;
    int ADD_PCT;
    int REMOVE_PCT;
    int SWAP_PCT;
    int INV_PCT;
    int SCR_PCT;
    double extra_param;
} WorkerParam;

// Thread-pool globals
static WorkerParam* task_array = NULL;
static volatile LONG task_count = 0;
static volatile LONG tasks_index = 0;
static volatile LONG tasks_done = 0;
static HANDLE* worker_handles = NULL;
static int num_workers = 0;
static volatile int pool_shutdown = 0;
static HANDLE task_event = NULL; // event to signal worker threads

static unsigned __stdcall pool_worker(void* arg)
{
    (void)arg;
    while (!pool_shutdown) {
        // wait until main signals tasks are available or shutdown
        WaitForSingleObject(task_event, INFINITE);
        if (pool_shutdown) break;
        // consume available tasks
        for (;;) {
            long idx = InterlockedIncrement(&tasks_index) - 1;
            if (idx >= task_count) break;
            WorkerParam* p = &task_array[idx];
            EvolveSpecie(
                p->ind, p->POP_SIZE,
                (const double**)p->dist,
                (const int**)p->dist_ranking,
                p->node_vector, p->total_stations,
                p->ALPHA, p->total_stations,
                p->MUTATION_RATE, p->ELITISM,
                p->ADD_PCT, p->REMOVE_PCT, p->SWAP_PCT,
                p->INV_PCT, p->SCR_PCT, p->extra_param);
            InterlockedIncrement(&tasks_done);
        }
        // finished consuming tasks; worker will wait again
    }
    return 0;
}

// Initialize pool with given worker count
static int pool_init(int workers)
{
    if (workers <= 0) return 0;
    num_workers = workers;
    worker_handles = (HANDLE*)malloc(num_workers * sizeof(HANDLE));
    if (!worker_handles) return 0;
    // create manual-reset event; initially non-signaled
    task_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!task_event) { free(worker_handles); worker_handles = NULL; num_workers = 0; return 0; }
    for (int i = 0; i < num_workers; ++i) {
        unsigned threadId;
        worker_handles[i] = (HANDLE)_beginthreadex(NULL, 0, pool_worker, NULL, 0, &threadId);
        if (!worker_handles[i]) {
            // cleanup
            for (int j = 0; j < i; ++j) CloseHandle(worker_handles[j]);
            CloseHandle(task_event); task_event = NULL;
            free(worker_handles); worker_handles = NULL; num_workers = 0; return 0;
        }
    }
    return 1;
}

static void pool_shutdown_and_wait(void)
{
    pool_shutdown = 1;
    // wake all workers so they can exit
    if (task_event) SetEvent(task_event);
    for (int i = 0; i < num_workers; ++i) {
        WaitForSingleObject(worker_handles[i], INFINITE);
        CloseHandle(worker_handles[i]);
    }
    free(worker_handles); worker_handles = NULL; num_workers = 0;
    if (task_event) { CloseHandle(task_event); task_event = NULL; }
}

// Add evaluation worker globals
static volatile LONG eval_index = 0;

typedef struct {
    Individual** species;
    int NUM_SPECIES;
    int POP_SIZE;
    double** dist;
    int** dist_ranking;
    int total_stations;
    int ALPHA;
    SpeciesCostLocal* out;
} EvalArgs;

static unsigned __stdcall EvalWorker(void* arg)
{
    EvalArgs* a = (EvalArgs*)arg;
    if (!a) return 0;
    for (;;) {
        long idx = InterlockedIncrement(&eval_index) - 1;
        if (idx >= a->NUM_SPECIES) break;
        int s = (int)idx;
        double* costs = Total_Cost_Specie(a->ALPHA, a->species[s], a->POP_SIZE, a->total_stations, (const double**)a->dist, (const int**)a->dist_ranking);
        if (!costs) {
            a->out[s].cost = 1e18;
            a->out[s].best_idx = -1;
        } else {
            int best_idx = Select_Best(costs, a->POP_SIZE);
            a->out[s].cost = costs[best_idx];
            a->out[s].best_idx = best_idx;
            free(costs);
        }
        a->out[s].species_id = s;
        InterlockedIncrement(&tasks_done); // reuse tasks_done as completed-counter
    }
    return 0;
}

// -------------------------------------------------------------

int main(int argc, char** argv)
{
    clock_t start_total = clock();

    printf("[MAIN] Starting program...\n");

    // initialize critical section for thread synchronization
    InitializeCriticalSection(&evolve_cs);

    int max_generations = 10000; // default
    int LOG_INTERVAL = 50; // report less frequently to reduce overhead
    int verbose = 0;

    // GA parameters (keep same logic as C++ original)
    int NUM_SPECIES = 100;
    int POP_SIZE = 50;
    double MUTATION_RATE = 0.25;
    const int ALPHA = 3;
    const int ELITISM = 0;

    int ADD_PCT = 15;
    int REMOVE_PCT = 10;
    int SWAP_PCT = 15;
    const int INV_PCT = 5;
    const int SCR_PCT = 5;

    int requested_workers = 0; // default 0 = no threading

    // parse command-line and update values
    parse_args(argc, argv, &max_generations, &LOG_INTERVAL, &NUM_SPECIES, &POP_SIZE, &requested_workers, &verbose);

    print_now("Starting dataset load");

    // Load dataset
    int total_stations = 0;
    Node* node_vector = readDataset("data\\127\\127_data.txt", &total_stations);
    if (!node_vector || total_stations == 0) {
        fprintf(stderr, "Error: could not read dataset\n");
        return 1;
    }

    print_now("Dataset loaded");
    fflush(stdout);
    if (verbose) { printf("  Nodes loaded: %d\n", total_stations); fflush(stdout); }

    // Compute distance matrix
    print_now("Computing distance matrix"); fflush(stdout);
    clock_t t0 = clock();
    double** dist = Compute_Distances_2DVector(node_vector, total_stations);
    clock_t t1 = clock();
    if (!dist) {
        fprintf(stderr, "Error: distance computation failed\n"); fflush(stderr);
        free(node_vector);
        return 1;
    }
    if (verbose) { printf("  Distance matrix computed in %ld ms\n", (long)((t1 - t0) * 1000 / CLOCKS_PER_SEC)); fflush(stdout); }

    // Compute ranking
    print_now("Computing distance ranking"); fflush(stdout);
    t0 = clock();
    int** dist_ranking = Distance_Ranking_2DVector(dist, total_stations, total_stations);
    t1 = clock();
    if (!dist_ranking) {
        fprintf(stderr, "Error: ranking computation failed\n"); fflush(stdout);
        Free_2DArray_Double(dist, total_stations);
        free(node_vector);
        return 1;
    }
    if (verbose) { printf("  Ranking computed in %ld ms\n", (long)((t1 - t0) * 1000 / CLOCKS_PER_SEC)); fflush(stdout); }

    RNG_Init();

    // Initialize species
    print_now("Initializing population");
    t0 = clock();
    int species_count = 0;
    Individual** species = Random_Generation(node_vector, total_stations, NUM_SPECIES, POP_SIZE, &species_count);
    t1 = clock();
    if (!species) {
        fprintf(stderr, "Error: population initialization failed\n");
        Free_2DArray_Int(dist_ranking, total_stations);
        Free_2DArray_Double(dist, total_stations);
        free(node_vector);
        return 1;
    }
    if (verbose) printf("  Population initialized in %ld ms\n", (long)((t1 - t0) * 1000 / CLOCKS_PER_SEC));

    // Start thread pool if requested
    if (requested_workers > 0) {
        int ok = pool_init(requested_workers);
        if (!ok) {
            fprintf(stderr, "Warning: thread pool initialization failed, running single-threaded\n");
            requested_workers = 0;
        } else {
            num_workers = requested_workers;
            if (verbose) printf("Thread pool started with %d workers\n", num_workers);
        }
    }

    print_now("Starting evolution loop");

    double old_best = 1e18;
    int stagnation_count = 0;
    double base_mutation_rate = MUTATION_RATE;
    int stagnation_patience = 50; // increased patience

    for (int gen = 0; gen < max_generations; ++gen) {
        int log_detailed = (gen % LOG_INTERVAL == 0);
        clock_t gen_start = clock();

        // Evolve all species and measure evolution time
        clock_t evolve_start = clock();
        long evolve_time = 0;
        // Use thread-pool if initialized, otherwise run serial
        if (num_workers > 0) {
            // build tasks
            if (task_array) { free(task_array); task_array = NULL; }
            task_array = (WorkerParam*)malloc(NUM_SPECIES * sizeof(WorkerParam));
            task_count = NUM_SPECIES; tasks_index = 0; tasks_done = 0; pool_shutdown = 0;
            for (int s = 0; s < NUM_SPECIES; ++s) {
                task_array[s].ind = species[s];
                task_array[s].POP_SIZE = POP_SIZE;
                task_array[s].dist = dist;
                task_array[s].dist_ranking = dist_ranking;
                task_array[s].node_vector = node_vector;
                task_array[s].total_stations = total_stations;
                task_array[s].ALPHA = ALPHA;
                task_array[s].MUTATION_RATE = MUTATION_RATE;
                task_array[s].ELITISM = ELITISM;
                task_array[s].ADD_PCT = ADD_PCT;
                task_array[s].REMOVE_PCT = REMOVE_PCT;
                task_array[s].SWAP_PCT = SWAP_PCT;
                task_array[s].INV_PCT = INV_PCT;
                task_array[s].SCR_PCT = SCR_PCT;
                task_array[s].extra_param = 0.5;
            }
            // signal workers that tasks are available
            InterlockedExchange(&tasks_done, 0);
            InterlockedExchange(&tasks_index, 0);
            // set the event to wake workers
            SetEvent(task_event);
            // wait for workers to finish tasks
            while (tasks_done < task_count) Sleep(1);
            // reset event so workers will wait next time
            ResetEvent(task_event);
            clock_t evolve_end = clock();
            evolve_time = (long)((evolve_end - evolve_start) * 1000 / CLOCKS_PER_SEC);
        } else {
            // serial fallback: evolve species one by one
            for (int s = 0; s < NUM_SPECIES; ++s) {
                EvolveSpecie(
                    species[s], POP_SIZE,
                    (const double**)dist,
                    (const int**)dist_ranking,
                    node_vector, total_stations,
                    ALPHA, total_stations,
                    MUTATION_RATE, ELITISM,
                    ADD_PCT, REMOVE_PCT, SWAP_PCT,
                    INV_PCT, SCR_PCT, 0.5);
            }
            clock_t evolve_end = clock();
            evolve_time = (long)((evolve_end - evolve_start) * 1000 / CLOCKS_PER_SEC);
        }

        // Detailed reporting every LOG_INTERVAL generations
        if (log_detailed) {
            evaluate_and_report(gen, NUM_SPECIES, POP_SIZE, dist, dist_ranking, node_vector, total_stations, ALPHA, species, &old_best, &stagnation_count, &MUTATION_RATE);
            if (stagnation_count > stagnation_patience) {
                printf("  [STAGNATION] stopping early at generation %d\n", gen);
                break;
            }
        }

        if (verbose) {
            clock_t gen_end = clock();
            printf("  Generation %d took %ld ms\n", gen, (long)((gen_end - gen_start) * 1000 / CLOCKS_PER_SEC));
        }
    }

    // Final reporting (same logic as C++)
    double final_best_cost = 1e18;
    int final_best_species = 0;
    int final_best_index = 0;

    for (int s = 0; s < NUM_SPECIES; ++s) {
        double* costs = Total_Cost_Specie(ALPHA, species[s], POP_SIZE, total_stations, (const double**)dist, (const int**)dist_ranking);
        int best_idx = Select_Best(costs, POP_SIZE);
        if (costs[best_idx] < final_best_cost) {
            final_best_cost = costs[best_idx];
            final_best_species = s;
            final_best_index = best_idx;
        }
        free(costs);
    }

    printf("Final best cost: %.2f (species %d, idx %d)\n", final_best_cost, final_best_species, final_best_index);

    // Cleanup
    Free_Population(species, NUM_SPECIES, POP_SIZE);
    Free_2DArray_Int(dist_ranking, total_stations);
    Free_2DArray_Double(dist, total_stations);
    free(node_vector);
    if (task_array) { free(task_array); task_array = NULL; }

    // before exiting program, delete critical section and shutdown pool
    if (num_workers > 0) {
        pool_shutdown_and_wait();
    }
    DeleteCriticalSection(&evolve_cs);
    return 0;
}
